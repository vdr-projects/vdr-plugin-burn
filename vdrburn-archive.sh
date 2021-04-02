#!/bin/bash

# For DEBUG Output - can be left since logfiles are deleted when job finishes
set -x

# To make the script exit whenever a command fails (MUST NOT BE REMOVED)
set -e

# Some versions of growisofs refuse to start when run with sudo, and may
# misdetect this condition if vdr itself is started from a sudo session.
unset SUDO_COMMAND

echo "Script invokation: $*"

case $1 in
	mkiso)
		mkisofs -V "$DISC_ID" -f -r -J -l -allow-lowercase -allow-multidot \
				-graft-points $GRAFT_POINTS >"$ISO_FILE"
	;;

	burndir)
		SPEED=""
		if [ $BURN_SPEED -gt 0 ]; then
			SPEED="-speed=$BURN_SPEED"
		fi
		growisofs -use-the-force-luke=tty $SPEED -Z $DVD_DEVICE \
				  -V "$DISC_ID" -f -r -J -l -allow-lowercase -allow-multidot \
				  -graft-points $GRAFT_POINTS
	;;

	pipeiso)
		mkisofs -V "$DISC_ID" -f -r -J -l -allow-lowercase -allow-multidot \
				-graft-points $GRAFT_POINTS | tee "$ISO_FILE" > $ISO_PIPE
	;;

	burniso)
		SPEED=""
		if [ $BURN_SPEED -gt 0 ]; then
			SPEED="-speed=$BURN_SPEED"
		fi
		growisofs -use-the-force-luke=tty $SPEED -dvd-compat \
				  -Z $DVD_DEVICE=$ISO_PIPE
	;;

	recordingmark)
		cd "$RECORDING_PATH"
		RECDIR=$(basename $(pwd))
		cd ..
		UPPERRECDIR=$(basename $(pwd))
		cd ..
		RECPATH=$(pwd)
		
		RECORDING_DMH="$RECPATH"/"$UPPERRECDIR"_DVD/"$RECDIR"
		
		mkdir -p "$RECORDING_DMH"
		cp "$CONFIG_PATH/counters/standard" "$RECORDING_DMH"/dvd.vdr
		# Old style Records ("index.vdr"/"info.vdr")
		if [ -f "$RECORDING_PATH"/index.vdr ]; then
			cp "$RECORDING_PATH"/info.vdr "$RECORDING_DMH"/info.vdr
			cp "$RECORDING_PATH"/index.vdr "$RECORDING_DMH"/index.vdr
		# New style Records ("index"/"info")
		elif [ -f "$RECORDING_PATH"/index ]; then
			cp "$RECORDING_PATH"/info "$RECORDING_DMH"/info
			cp "$RECORDING_PATH"/index "$RECORDING_DMH"/index
		fi
	;;

	archivemark)
		((DVD_ID = `cat $CONFIG_PATH/counters/standard | sed "s/^0*//"` + 1))
		printf "%04d\n" $DVD_ID > "$CONFIG_PATH/counters/standard"
	;;

	*)
		echo "Usage: $0 {demux}"
		exit 1
	;;
esac

exit $?
