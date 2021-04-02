#!/bin/bash

# For DEBUG Output - can be left since logfiles are deleted when job finishes
set -x

# To make the script exit whenever a command fails (MUST NOT BE REMOVED)
set -e

# TODO
# Variables passed in (not up-to-date):

# RECORDING_PATH	(demux, recordingmark, dmharchive)
#					Path of VDR recording (/video0/%Bla/2004-01-...rec)
# MPEG_TMP_PATH		(demux, mplex)
#					a linux filesystem where fifos are created
# MPEG_DATA_PATH	(demux, mplex)
#					Path where streams and movie are stored
# VIDEO_FILE		(requant, mplex)
#					Full path- and filename of demuxed video stream
# AUDIO_FILES		(mplex)
#					Space-separated list of demuxed audio streams
# MOVIE_FILE		(mplex)
#					Full path- and filename of muxed movie
# REQUANT_FACTOR	(requant)
#					Factor by that video shall be shrinked
# IGNORE_TRACKS		(demux)
#					Comma-separated list of audio track IDs to be ignored
# USED_TRACKS		(demux)
#					Comma-separated list of audio track IDs to be used
# DVDAUTHOR_XML		(author)
#					Full path- and filename of the DVDAuthor XML
# DVDAUTHOR_PATH	(author, burn, dmharchive, archivemark)
#					Full path of the written DVD structure
# TRACK_ON_DVD		(dmharchive)
#					Number of track on dvd in which VDR recording is saved (2 digits)

# ISO_FILE			(burn, pipe, author)
#					Full path- and filename of target ISO
# ISO_PIPE			(burn, pipe)
#					Fifo where the created ISO should be piped into
# DVD_DEVICE		(burn, author)
#					Full path- and filename of the DVD burner device
# CONFIG_PATH			(pipe, recordingmark, archivemark, demux, dmharchive)
#					Full path to burn's config directory
# TEMP_PATH			(dmharchive)
#					Full path to burn's temp directory (namely the same as $MPEG_TMP_PATH)

# GRAFT_POINTS		(burn, pipe)
# DISC_ID			(burn, pipe)
# BURN_SPEED		(burn)
# USE_CUTTING		(demux)
# REQUANT_FILE		(requant)
# VIDEO_SIZE		(requant)
# MENU_BACKGROUND	(render)
# MENU_SOUNDTRACK	(render)
# MENU_XML			(render)
# MENU_M2V			(render)
# MENU_MPEG			(render)





if [ -z $JAVA_HOME ]; then
	export JAVA_HOME=/opt/j2re1.4.2
fi

if [ -z $PROJECTX_HOME ]; then
	export PROJECTX_HOME=/opt/ProjectX
fi

# Some versions of growisofs refuse to start when run with sudo, and may
# misdetect this condition if vdr itself is started from a sudo session.
unset SUDO_COMMAND

echo "Script invokation: $*"

IO_NICE=$(which ionice >/dev/null 2>&1 && echo "ionice -c 3")

case $1 in
	render)
		ASPECT="-a 3" # 16:9
		[ "z$MENU_ASPECT" = "z4:3" ] && ASPECT="-a 2" # 4:3

		png2yuv -j "$MENU_BACKGROUND" -f 25 -n 1 -I t -L 1 | \
		mpeg2enc -f 8 -n p $ASPECT -o "$MENU_M2V"
		mplex -f 8 -o /dev/stdout "$MENU_M2V" "$MENU_SOUNDTRACK" | \
		spumux -v 2 "$MENU_XML" > "$MENU_MPEG"
	;;

	demux)
		test -e "$MPEG_TMP_PATH/convert" && rm "$MPEG_TMP_PATH/convert"
		ln -s "$RECORDING_PATH" "$MPEG_TMP_PATH/convert"

		CUT=""
		if [ ! -z $USE_CUTTING ]; then
			CUT="-cut $MPEG_DATA_PATH/px.cut"
		fi

		$IO_NICE $JAVA_HOME/bin/java -Djava.awt.headless=true \
				-jar $PROJECTX_HOME/ProjectX.jar \
				-ini $CONFIG_PATH/ProjectX.ini \
				$TTXT_OPTS \
				$CUT -id $USED_TRACKS \
				-demux -out "$MPEG_DATA_PATH" -name vdrsync \
				$(ls "$MPEG_TMP_PATH/convert/"[0-9][0-9][0-9]*.*)
	;;

	requant)
		REQUANT_FACTOR=$(echo "$REQUANT_FACTOR" | tr ',' '.')
		echo requant $REQUANT_FACTOR 3 $VIDEO_SIZE
		$IO_NICE requant $REQUANT_FACTOR 3 $VIDEO_SIZE < "$VIDEO_FILE" > "$REQUANT_FILE"
		rm -f "$VIDEO_FILE"
	;;

	tcrequant)
		REQUANT_FACTOR=$(echo "$REQUANT_FACTOR" | tr ',' '.')
		echo tcrequant -f $REQUANT_FACTOR
		$IO_NICE tcrequant -f $REQUANT_FACTOR < "$VIDEO_FILE" > "$REQUANT_FILE"
		rm -f "$VIDEO_FILE"
	;;

	lxrequant)
		echo requant_lxdvdrip -f $REQUANT_FACTOR
		$IO_NICE requant_lxdvdrip -f $REQUANT_FACTOR -i "$VIDEO_FILE" -o "$REQUANT_FILE"
		rm -f "$VIDEO_FILE"
	;;

	mplex)
		MPLEX_OPTS="-S 0"
		(mplex -h 2>&1 | grep -q -- --ignore-seqend-markers) && \
			MPLEX_OPTS="$MPLEX_OPTS -M"

		#echo mplex -f 8 $MPLEX_OPTS -o "$MOVIE_FILE" "$VIDEO_FILE" $AUDIO_FILES
		$IO_NICE mplex -f 8 $MPLEX_OPTS -o "$MOVIE_FILE" "$VIDEO_FILE" $AUDIO_FILES
		
		rm -f "$VIDEO_FILE" $AUDIO_FILES
	;;

	subtitle)
		#echo SPUMUX_FILE $SPUMUX_FILE
		#echo MOVIE_FILE $MOVIE_FILE
		#echo SUBTITLED_FILE $SUBTITLED_FILE
		#echo NUMBER $NUMBER
		#echo TTXTPAGE $TTXTPAGE

		SON=$(find "$MPEG_DATA_PATH" -name \*${TTXTPAGE}\*.son)
 		SRT=$(find "$MPEG_DATA_PATH" -name \*${TTXTPAGE}\*.srt)
		SUP=$(find "$MPEG_DATA_PATH" -name \*${TTXTPAGE}\*.sup)
		if [ "!" "x$SON" = "x" ]; then
			echo found SON files: $SON
			find "$MPEG_DATA_PATH" -name \*${TTXTPAGE}\*.son | while read SPUFILE ; do
				# spumux.xml generation is based on son2spumux.sh: http://brigitte.dna.fi/~apm/
				echo "<subpictures>" > "$SPUMUX_FILE"
				echo "  <stream>" >> "$SPUMUX_FILE"
				cat "$SPUFILE" | tail -n +11 | while read l1
				do
					read l2 || exit 1
					x=`echo $l1 | cut -f2 -d\( | awk '{printf("%d", $1);}'`
					y=`echo $l1 | cut -f2 -d\( | awk '{printf("%d", $2);}'`
					t1=`echo $l2 | awk '{t1=substr($2,1,8); t2=substr($2,10,2); printf("%s.%s", t1, t2);}'`
					t2=`echo $l2 | awk '{t1=substr($3,1,8); t2=substr($3,10,2); printf("%s.%s", t1, t2);}'`
					i=`echo $l2 | awk '{printf("%s", $NF);}'`
					echo "    <spu start=\"$t1\"" >> "$SPUMUX_FILE"
					echo "         end=\"$t2\"" >> "$SPUMUX_FILE"
					echo "         image=\"$MPEG_DATA_PATH/$i\"" >> "$SPUMUX_FILE"
					echo "         xoffset=\"$x\" yoffset=\"$y\"" >> "$SPUMUX_FILE"
					echo "         transparent=\"000060\" />" >> "$SPUMUX_FILE"
				done
				echo "  </stream>" >> "$SPUMUX_FILE"
				echo "</subpictures>" >> "$SPUMUX_FILE"
			done
 		elif [ "!" "x$SRT" = "x" ]; then
			echo found SRT files: $SRT
 			echo "<subpictures>" > "$SPUMUX_FILE"
 			echo "  <stream>" >> "$SPUMUX_FILE"
 			echo "         <textsub filename=\"$SRT\" characterset=\"ISO8859-1\"" >> "$SPUMUX_FILE"
 			echo "         fontsize=\"28.0\" font=\"arial.ttf\" horizontal-alignment=\"center\"" >> "$SPUMUX_FILE"
 			echo "         vertical-alignment=\"bottom\" left-margin=\"60\" right-margin=\"60\"" >> "$SPUMUX_FILE"
 			echo "         top-margin=\"20\" bottom-margin=\"30\" subtitle-fps=\"25\"" >> "$SPUMUX_FILE"
 			echo "         movie-fps=\"25\" movie-width=\"720\" movie-height=\"574\"" >> "$SPUMUX_FILE"
 			echo "      />" >> "$SPUMUX_FILE"
 			echo "  </stream>" >> "$SPUMUX_FILE"
 			echo "</subpictures>" >> "$SPUMUX_FILE"
		elif [ "!" "x$SUP" = "x" ]; then
			echo found SUP files: $SUP
			pxsup2dast "${SUP}" "${SUP}.IFO"
			mv "${SUP%sup}d/spumux.xml" "$SPUMUX_FILE"
		fi

		$IO_NICE spumux -s $NUMBER -v 2 "$SPUMUX_FILE" < "$MOVIE_FILE" > "$SUBTITLED_FILE"
	;;

	author)
		$IO_NICE dvdauthor -x "$DVDAUTHOR_XML"
	;;

	dmharchive)
		echo "Creating $TEMP_PATH/INDEX_${TRACK_ON_DVD}"
		mkdir -p "$TEMP_PATH/INDEX_${TRACK_ON_DVD}"
		cd "$DVDAUTHOR_PATH/VIDEO_TS/"
		for i in $(seq 1 99) ; do
			if [ -f "VTS_${TRACK_ON_DVD}_$i.VOB" ] ; then
				vdr_file=`printf "%03d.vdr" $i`
				echo "Linking $DVDAUTHOR_PATH/VIDEO_TS/VTS_${TRACK_ON_DVD}_$i.VOB -> $TEMP_PATH/INDEX_${TRACK_ON_DVD}/$vdr_file"
				ln -s "$DVDAUTHOR_PATH/VIDEO_TS/VTS_${TRACK_ON_DVD}_$i.VOB" "$TEMP_PATH/INDEX_${TRACK_ON_DVD}/$vdr_file"
			else
				break
			fi
		done
		cd "$TEMP_PATH/INDEX_${TRACK_ON_DVD}"
		genindex
		if [ "$?" != "0" ] ; then
			exit 1
		fi
		
		cd "$RECORDING_PATH"
		RECDIR=$(basename $(pwd))
		cd ..
		UPPERRECDIR=$(basename $(pwd))
		cd ..
		RECPATH=$(pwd)
		RECORDING_DMH="$RECPATH"/"$UPPERRECDIR"_DVD/"$RECDIR"
		
		mkdir -p "$RECORDING_DMH"
		echo "Copying $TEMP_PATH/INDEX_${TRACK_ON_DVD}/index.vdr -> "$RECORDING_DMH"/index"
		cp "$TEMP_PATH/INDEX_${TRACK_ON_DVD}/index.vdr" "$RECORDING_DMH"/index
		echo "Moving $TEMP_PATH/INDEX_${TRACK_ON_DVD}/index.vdr -> $DVDAUTHOR_PATH/VIDEO_TS/index_${TRACK_ON_DVD}.vdr"
		mv "$TEMP_PATH/INDEX_${TRACK_ON_DVD}/index.vdr" "$DVDAUTHOR_PATH/VIDEO_TS/index_${TRACK_ON_DVD}.vdr"
		cp "$RECORDING_PATH"/info "$RECORDING_DMH"/info
		if [ -f "$RECORDING_PATH/info" ]; then
			echo "Copying $RECORDING_PATH/info -> $DVDAUTHOR_PATH/VIDEO_TS/info_${TRACK_ON_DVD}.vdr"
			cp "$RECORDING_PATH/info" "$DVDAUTHOR_PATH/VIDEO_TS/info_${TRACK_ON_DVD}.vdr"
		fi
		echo "Deleting $TEMP_PATH/INDEX_${TRACK_ON_DVD}"
		rm -rf "$TEMP_PATH/INDEX_${TRACK_ON_DVD}"
		echo "Creating "$RECORDING_DMH"/dvd.vdr"
		cp "$CONFIG_PATH/counters/standard" "$RECORDING_DMH"/dvd.vdr
		printf "%04d\n" $(echo ${TRACK_ON_DVD} | sed 's/^0*//') >> "$RECORDING_DMH"/dvd.vdr
	;;

	archivemark)
		echo "Creating $DVDAUTHOR_PATH/dvd.vdr"
		cp "$CONFIG_PATH/counters/standard" "$DVDAUTHOR_PATH/dvd.vdr"
		((DVD_ID = `cat $CONFIG_PATH/counters/standard | sed "s/^0*//"` + 1))
		printf "%04d\n" $DVD_ID > "$CONFIG_PATH/counters/standard"
	;;

	mkiso)
		$IO_NICE mkisofs -V "$DISC_ID" -dvd-video "$DVDAUTHOR_PATH" > "$ISO_FILE"
	;;

	burndir)
		SPEED=""
		if [ $BURN_SPEED -gt 0 ]; then
			SPEED="-speed=$BURN_SPEED"
		fi
		$IO_NICE growisofs -use-the-force-luke=tty $SPEED -dvd-compat -Z "$DVD_DEVICE" \
				  -V "$DISC_ID" -dvd-video "$DVDAUTHOR_PATH"
	;;

	pipeiso)
		$IO_NICE mkisofs -V "$DISC_ID" -dvd-video "$DVDAUTHOR_PATH" \
			| tee "$ISO_FILE" > "$ISO_PIPE"
	;;

	burniso)
		SPEED=""
		if [ $BURN_SPEED -gt 0 ]; then
			SPEED="-speed=$BURN_SPEED"
		fi
		$IO_NICE growisofs -use-the-force-luke=tty $SPEED -dvd-compat \
				  -Z "$DVD_DEVICE=$ISO_PIPE"
	;;

	*)
		echo "Usage: $0 {demux}"
		exit 1
	;;
esac

exit $?
