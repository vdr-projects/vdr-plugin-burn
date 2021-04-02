/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: i18n.c,v 1.41 2006/10/24 17:47:04 lordjaxom Exp $
 */

#include <vdr/config.h>

#if APIVERSNUM < 10507

#include "i18n.h"

namespace vdr_burn
{

	// --- i18n ---------------------------------------------------------------

	i18n::i18n():
			m_osdLanguage(-1)
	{
	}

	i18n& i18n::get()
	{
		static i18n instance;
		return instance;
	}

	// --- phrases ------------------------------------------------------------

	const tI18nPhrase i18n::m_phrases[] = {
		/* Template
		{ "Template",
		  "Vorlage",
		  "",
		  "",
		  "",
		  "Mod�le",
		  "",
		  "Mallipohja",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		}, */
		{ "Create DVDs",
		  "DVDs erstellen",
		  "",
		  "",
		  "",
		  "",
		  "Graver des DVD",
		  "",
		  "Polta DVD-levy",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� DVD",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Versatile convert-and-burn plugin",
		  "Universelles Umwandlungs- und Brennplugin",
		  "",
		  "",
		  "",
		  "",
		  "Plugin convertiseur et graveur universel",
		  "",
		  "Monipuolinen DVD-ty�kalu",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� DVD �� ������� VDR",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "No recordings marked",
		  "Keine Aufzeichnungen markiert",
		  "",
		  "",
		  "",
		  "",
		  "Pas d'enregistrements marqu�s pour la gravure",
		  "",
		  "Ei valittuja tallenteita",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��� �������� �������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "$Total size: %1$.1f MB %2$s",
		  "Gesamtgr��e: %1$.1f MB %2$s",
		  "",
		  "",
		  "",
		  "",
		  "Volume total: %1$.1f MO %2$s",
		  "",
		  "Yhteens�: %1$.1f MB %2$s",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����� ������: %1$.1f �� %2$s",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Remove recording from list?",
		  "Aufzeichnung aus Liste entfernen?",
		  "",
		  "",
		  "",
		  "",
		  "Enlever l'enregistrement de la liste?",
		  "",
		  "Poistetaanko tallenne listalta?",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� ������ �� ������?",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Remove",
		  "Entfernen",
		  "",
		  "",
		  "",
		  "",
		  "Enlever",
		  "",
		  "Poista",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Button$Start",
		  "Starten",
		  "",
		  "",
		  "",
		  "",
		  "Graver",
		  "",
		  "Aloita",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "(would be shrinked)",
		  "(w�rden geschrumpft)",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "(joudutaan pienent�m��n)",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "(����� �����)",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Status",
		  "Status",
		  "",
		  "",
		  "",
		  "",
		  "Status",
		  "",
		  "Status",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "No active or waiting jobs present",
		  "Keine aktiven oder wartenden Auftr�ge vorhanden",
		  "",
		  "",
		  "",
		  "",
		  "Pas de t�ches o� d'activ�s pr�sente",
		  "",
		  "Ei aktiivisia eik� odottavia teht�vi�",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��� ����������� ��� ������ �������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "List",
		  "Liste",
		  "",
		  "",
		  "",
		  "",
		  "Liste",
		  "",
		  "Lista",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Erroneous jobs",
		  "Fehlerhafte Auftr�ge",
		  "",
		  "",
		  "",
		  "",
		  "T�ches erron�e",
		  "",
		  "Ep�onnistuneet teht�v�t",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������, ���������� �� ������� ������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Disk type",
		  "Art der DVD",
		  "",
		  "",
		  "",
		  "",
		  "Genre de DVD",
		  "",
		  "Levyn tyyppi",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "DVD with menus",
		  "DVD mit Men�s",
		  "",
		  "",
		  "",
		  "",
		  "DVD avec menu",
		  "",
		  "DVD valikolla",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "DVD � ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Archive disk",
		  "Archiv-Disk",
		  "",
		  "",
		  "",
		  "",
		  "Disque archive",
		  "",
		  "arkistolevy",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Please insert an empty disk and press Ok",
		  "Bitte legen Sie eine leere DVD ein und dr�cken Sie Ok",
		  "",
		  "",
		  "",
		  "",
		  "Veuilliez introduire un DVD vierge et appuyez sur OK",
		  "",
		  "Aseta tyhj� levy asemaan ja paina 'OK'",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� ������ ���� DVD � ������� ��",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Cancel",
		  "Abbrechen",
		  "",
		  "",
		  "",
		  "",
		  "Interrompre",
		  "",
		  "Peru",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Play movie",
		  "Film abspielen",
		  "",
		  "",
		  "",
		  "",
		  "Regarder le film",
		  "",
		  "Toista tallenne",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Job still active - really cancel?",
		  "Auftrag noch aktiv - wirklich beenden?",
		  "",
		  "",
		  "",
		  "",
		  "T�che encore active - vraiment int�rompre?",
		  "",
		  "Teht�v�� suoritetaan yh� - perutaanko?",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� ����������� - ������������� ��������?",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "$Job active (Writing: %1$d%%)",
		  "Auftrag aktiv (Schreibe: %1$d%%)",
		  "",
		  "",
		  "",
		  "",
		  "Travail actif (Gravure:  %1$d%%)",
		  "",
		  "Teht�v�� suoritetaan (poltto: %1$d%%)",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� ����������� (������: %1$d%%)",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Job active (Converting)",
		  "Auftrag aktiv (Konvertiere)",
		  "",
		  "",
		  "",
		  "",
		  "T�che active (convertion)",
		  "",
		  "Teht�v�� suoritetaan (konvertointi)",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� ����������� (�����������)",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "DVD title",
		  "Titel der DVD",
		  "",
		  "",
		  "",
		  "",
		  "Titre du DVD",
		  "",
		  "DVD:n nimi",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� DVD",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Move down",
		  "Nach unten",
		  "",
		  "",
		  "",
		  "",
		  "Vers en bas",
		  "",
		  "Alasp�in",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Next page",
		  "N�chste Seite",
		  "",
		  "",
		  "",
		  "",
		  "La prochaine page",
		  "",
		  "Seuraava sivu",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Previous page",
		  "Vorherige Seite",
		  "",
		  "",
		  "",
		  "",
		  "La page pr�c�dante",
		  "",
		  "Edellinen sivu",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Target",
		  "Ziel",
		  "",
		  "",
		  "",
		  "",
		  "Cible",
		  "",
		  "Kohde",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Create ISO only",
		  "Nur Abbild erzeugen",
		  "",
		  "",
		  "",
		  "",
		  "Cr�er seulement l'image",
		  "",
		  "vain levyimage",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ ������� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Write to Disc",
		  "Nur brennen",
		  "",
		  "",
		  "",
		  "",
		  "Seulement graver",
		  "",
		  "vain poltto",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���� �� DVD",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Create ISO and write",
		  "Abbild erzeugen und brennen",
		  "",
		  "",
		  "",
		  "",
		  "Cr�er l'image et graver",
		  "",
		  "levyimage ja poltto",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� ����� � ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Chapters",
		  "Kapitel",
		  "",
		  "",
		  "",
		  "",
		  "Chapitre",
		  "",
		  "Kappaleet",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "No chapters",
		  "Keine Kapitel",
		  "",
		  "",
		  "",
		  "",
		  "Pas de chapitre",
		  "",
		  "ei kappaleita",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Use editing marks",
		  "Schnittmarken benutzen",
		  "",
		  "",
		  "",
		  "",
		  "Utiliser les marques de coupe",
		  "",
		  "leikkausmerkinn�t",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������������ ��������� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Every 5 minutes",
		  "Jede 5 Minuten",
		  "",
		  "",
		  "",
		  "",
		  "Toute les 5 minutes",
		  "",
		  "joka 5. minuutti",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ 5 �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Every 10 minutes",
		  "Jede 10 Minuten",
		  "",
		  "",
		  "",
		  "",
		  "Toute les 10 minutes",
		  "",
		  "joka 10. minuutti",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ 10 �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Every 15 minutes",
		  "Jede viertel Stunde",
		  "",
		  "",
		  "",
		  "",
		  "Tout les 1/4 d'heures",
		  "",
		  "joka 15. minuutti",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ 15 �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Every 30 minutes",
		  "Jede halbe Stunde",
		  "",
		  "",
		  "",
		  "",
		  "Toute les 1/2 heures",
		  "",
		  "joka 30. minuutti",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ 30 �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Every hour",
		  "Jede Stunde",
		  "",
		  "",
		  "",
		  "",
		  "Toutes les heures",
		  "",
		  "kerran tunnissa",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ ���",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Skin",
		  "Aussehen",
		  "",
		  "",
		  "",
		  "",
		  "Apparence",
		  "",
		  "Ulkoasu",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� ���",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer chapters",
		  "Kapitel anbieten",
		  "",
		  "",
		  "",
		  "",
		  "Proposer les chapitres",
		  "",
		  "Tarjoa kappaleita",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Disk size",
		  "Gr�sse der Disk",
		  "",
		  "",
		  "",
		  "",
		  "Taille du disque",
		  "",
		  "Levyn koko",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Custom disk size (MB)",
		  "Benutzerdef. Gr�sse (MB)",
		  "",
		  "",
		  "",
		  "",
		  "Taille d�finit par l'utilisateur (Mo)",
		  "",
		  "S��dett�v� levyn koko (MB)",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������������� ������ ����� (��)",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer disk size",
		  "Gr�sse der Disk anbieten",
		  "",
		  "",
		  "",
		  "",
		  "Proposer la taille du disque",
		  "",
		  "Tarjoa levyn kokoa",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ������ �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Single Layer",
		  "Single Layer",
		  "",
		  "",
		  "",
		  "",
		  "Simple couche",
		  "",
		  "yksikerros",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Double Layer",
		  "Double Layer",
		  "",
		  "",
		  "",
		  "",
		  "Double couche",
		  "",
		  "kaksikerros",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Custom",
		  "Benutzerdefiniert",
		  "",
		  "",
		  "",
		  "",
		  "D�finit par l'utilisateur",
		  "",
		  "s��dett�v�",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����������������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Choose",
		  "W�hlen",
		  "",
		  "",
		  "",
		  "",
		  "Choisir",
		  "",
		  "Valitse",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Analyzing recording...",
		  "Aufzeichnung wird analysiert...",
		  "",
		  "",
		  "",
		  "",
		  "Analyser l'enregistrement...",
		  "",
		  "Analysoidaan tallennetta...",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ������...",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Canceled jobs",
		  "Abgebrochene Auftr�ge",
		  "",
		  "",
		  "",
		  "",
		  "T�ches imterrompues",
		  "",
		  "Perutut teht�v�t",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� �������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Restart",
		  "�berarbeiten",
		  "",
		  "",
		  "",
		  "",
		  "Repris",
		  "",
		  "Aloita uudelleen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��������������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Default",
		  "Vorgabe",
		  "",
		  "",
		  "",
		  "",
		  "Par d�faut",
		  "",
		  "Oletus",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�� ���������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Replace current job?",
		  "Gegenw�rtigen Auftrag ersetzen?",
		  "",
		  "",
		  "",
		  "",
		  "Remplacer le travail actuel?",
		  "",
		  "Korvataanko nykyinen teht�v�?",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� ����������� �������?",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Retry",
		  "Wiederholen",
		  "",
		  "",
		  "",
		  "",
		  "R�p�ter",
		  "",
		  "Yrit� uudelleen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Remove job from list?",
		  "Auftrag aus Liste entfernen?",
		  "",
		  "",
		  "",
		  "",
		  "Enlever la t�che de la liste?",
		  "",
		  "Poistetaanko teht�v� listalta?",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� ������� �� ������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "A job failed!",
		  "Ein Auftrag ist fehlgeschlagen!",
		  "",
		  "",
		  "",
		  "",
		  "La t�che a echou�e!",
		  "",
		  "Polttaminen ep�onnistui!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ ��� �������� DVD!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "A job was finished successfully.",
		  "Ein Auftrag war erfolgreich.",
		  "",
		  "",
		  "",
		  "",
		  "La t�che est accomplie.",
		  "",
		  "Polttaminen onnistui.",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "DVD ������ �������.",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "No recordings marked for burning",
		  "Keine Aufzeichnungen zum Brennen markiert",
		  "",
		  "",
		  "",
		  "",
		  "Aucun enregistrement marqu� pour la gravure",
		  "",
		  "Tallenteita ei ole merkitty poltettavaksi",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��� ������� ��� �������� DVD",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "$Recordings: %1$d, total size: %2$.1f MB",
		  "Aufzeichnungen: %1$d, Gesamtgr��e: %2$.1f MB",
		  "",
		  "",
		  "",
		  "",
		  "Enregistrements: %1$d, Taille totale: %2$.1fMo",
		  "",
		  "Tallenteita: %1$d, yhteens�: %2$.1f MB",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������: %1$d, ����� ������: %2$.1f ��",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
                { "Video track",
        	  "Videospur",
		  "",
		  "",
		  "",
		  "",
		  "Piste vid�o",
		  "",
		  "Kuvaraita",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Audio track",
		  "Tonspur",
		  "",
		  "",
		  "",
		  "",
		  "Piste audio",
		  "",
		  "��niraita",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Unknown",
		  "Unbekannt",
		  "",
		  "",
		  "",
		  "",
		  "inconnue",
		  "",
		  "Tuntematon",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Remove path component",
		  "Pfad abschneiden",
		  "",
		  "",
		  "",
		  "",
		  "Couper de chemin",
		  "",
		  "Poista polku",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer target",
		  "Ziel anbieten",
		  "",
		  "",
		  "",
		  "",
		  "Proposer la cible",
		  "",
		  "Tarjoa kohdetta",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Burn speed",
		  "Brenngeschwindigkeit",
		  "",
		  "",
		  "",
		  "",
		  "Vitesse de gravure",
		  "",
		  "Polttonopeus",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� ������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "unlimited",
		  "unbegrenzt",
		  "",
		  "",
		  "",
		  "",
		  "sans limite",
		  "",
		  "ei rajoitusta",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "DVD without menus",
		  "DVD ohne Men�s",
		  "",
		  "",
		  "",
		  "DVD sans menu",
		  "",
		  "",
		  "DVD ilman valikkoa",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "DVD ��� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "CD-R",
		  "CD-R",
		  "",
		  "",
		  "",
		  "",
		  "CD-R",
		  "",
		  "CD-R",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "CD-R",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Back",
		  "zur�ck",
		  "",
		  "",
		  "",
		  "",
		  "retour",
		  "",
		  "Takaisin",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Burn job active",
		  "Brennjob l�uft",
		  "",
		  "",
		  "",
		  "",
		  "T�che de gravure est active",
		  "",
		  "Polttoteht�v� aktiivinen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "DVD ���������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Write DVDs",
		  "DVDs brennen",
		  "",
		  "",
		  "",
		  "",
		  "Gravure DVD",
		  "",
		  "Polta DVD-levy",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������� DVD",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Recordings",
		  "Aufzeichnungen",
		  "",
		  "",
		  "",
		  "",
		  "enregistrement",
		  "",
		  "tallenteet",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Job",
		  "Auftrag",
		  "",
		  "",
		  "",
		  "",
		  "T�che",
		  "",
		  "teht�v�",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Status",
		  "Status",
		  "",
		  "",
		  "",
		  "",
		  "Status",
		  "",
		  "status",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Cut",
		  "Schneiden",
		  "",
		  "",
		  "",
		  "",
		  "Couper",
		  "",
		  "Leikkaa",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer cutting",
		  "Schneiden anbieten",
		  "",
		  "",
		  "",
		  "",
		  "Proposer la coupe",
		  "",
		  "Tarjoa leikkausta",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Not enough free disk space!",
		  "Nicht genug Plattenplatz!",
		  "",
		  "",
		  "",
		  "",
		  "Pas assez d'espace disponible sur le disque!",
		  "",
		  "Liian v�h�n vapaata levytilaa!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������������ ����� �� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Demux using",
		  "Demultiplexer",
		  "",
		  "",
		  "",
		  "",
		  "Demultipl�xeur",
		  "",
		  "Demuksausohjelmisto",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "No video track selected!",
		  "Keine Videospur ausgew�hlt!",
		  "",
		  "",
		  "",
		  "",
		  "Pas de piste vid�o s�lectionn�e!",
		  "",
		  "Yht��n kuvaraitaa ei ole valittu!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�����-������� �� �������!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "No audio track selected!",
		  "Keine Tonspur ausgew�hlt!",
		  "",
		  "",
		  "",
		  "",
		  "Pas de piste audio s�lectionn�e!",
		  "",
		  "Yht��n ��niraitaa ei ole valittu!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�����-������� �� �������!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Recording too big for disc type!",
		  "Aufnahme zu gro� f�r Disk-Typ!",
		  "",
		  "",
		  "",
		  "",
		  "Enregistrement trop volumineux pour ce type de disque!",
		  "",
		  "Tallenne ei mahdu levytyypille!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ ������� ������� ��� ���������� ���� �����!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "VDRSync",
		  "VDRSync",
		  "",
		  "",
		  "",
		  "",
		  "VDRSync",
		  "",
		  "VDRSync",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "VDRSync",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "ProjectX",
		  "ProjectX",
		  "",
		  "",
		  "",
		  "",
		  "ProjectX",
		  "",
		  "ProjectX",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "ProjectX",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer DMH-archive",
		  "DMH-Archiv anbieten",
		  "",
		  "",
		  "",
		  "",
		  "Proposer l'archivage DMH",
		  "",
		  "Tarjoa DMH-arkistointia",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ����� DMH",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "DMH-archive",
		  "DMH-Archiv",
		  "",
		  "",
		  "",
		  "",
		  "Archivage DMH",
		  "",
		  "DMH-arkistointi",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����� DMH",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Archive-ID",
		  "Archiv-ID",
		  "",
		  "",
		  "",
		  "",
		  "ID-Archive",
		  "",
		  "Arkistotunnus",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "ID ������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Please enter the burn plugin",
		  "Bitte Brenn-Plugin aufrufen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Siirry \"Polta DVD-levy\"-laajennokseen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����������, ��������� ������ ��� �������� DVD",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Couldn't read index",
		  "Konnte Index nicht lesen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Indeksin lukeminen ep�onnistui!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�� ���� ��������� ������!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Couldn't browse recording",
		  "Konnte Aufzeichnung nicht durchsuchen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Tallenteen selaaminen ep�onnistui!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�� ���� ���������� ������!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Couldn't read marks",
		  "Konnte Marken nicht lesen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Merkint�jen lukeminen ep�onnistui!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�� ���� ��������� �����!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Recording is too short",
		  "Aufzeichnung ist zu kurz",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Tallenne on liian lyhyt",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "������ ������� ��������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Requant using",
		  "Verkleinern mit",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Requant-ohjelmisto",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��������� � �������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Preserve logfiles",
		  "Logfiles behalten",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "S�ilyt� lokitiedostot",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Language code",
		  "Sprachkennung",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Kielitunnus",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Aspect ratio",
		  "Seitenverh�ltnis",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Kuvasuhde",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����������� ������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Spare language code",
		  "Ersatzsprachkennung",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Oletuskielitunnus",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "�������� ��� �����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Track$On",
		  "An",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "P��ll�",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���.",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Track$Off",
		  "Aus",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Poissa",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����.",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Status in main menu",
		  "Status im Hauptmen�",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "N�yt� status p��valikossa",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "��������� � ������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "analog",
		  "analog",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "analoginen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "dolby",
		  "dolby",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "dolby",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "dolby",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "M2VRequantizer",
		  "M2VRequantizer",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "M2VRequantizer",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "M2VRequantizer",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Transcode",
		  "Transcode",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Transcode",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "Transcode",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Disc too big for disc type!",
		  "Disc zu gro� f�r diese Art!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
 		  "Levy ei mahdu levytyypille!",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���� ������� ����� ��� ����� ����!",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer disk type",
		  "Art der DVD anbieten",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
 		  "Tarjoa levyn tyyppi�",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ��� DVD",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "--- Common settings --------------------------------------------------",
		  "--- Allgemeine Einstellungen -----------------------------------------",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "--- Yleiset asetukset -----------------------------------------------",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "--- ����� ��������� ------------------------------------------------",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "--- Job menu settings ------------------------------------------------",
		  "--- Jobmen�-Einstellungen --------------------------------------------",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "--- Teht�v�valikon asetukset ----------------------------------------",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "--- ��������� ���� ������� -------------------------------------------",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "--- Job defaults -----------------------------------------------------",
		  "--- Standard Job-Einstellungen ---------------------------------------",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "--- Oletusasetukset teht�ville ---------------------------------------",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "--- ����������� ��������� ������� ------------------------------------",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Skip empty titlemenu",
		  "Leeres Titelmen� �berspringen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Ohita tyhj� nimikevalikko",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ������ ���� ��������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer skip titlemenu",
		  "Titelmen� �berspringen anbieten",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Tarjoa nimikevalikon ohitusta",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ������� ������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Job options",
		  "Jobeinstellungen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Teht�v�n asetukset",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "����� �������",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Skip short mainmenu",
		  "Kurzes Hauptmen� �berspringen",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Ohita lyhyt p��valikko",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ������ ������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ "Offer skip mainmenu",
		  "Hauptmen� �berspringen anbieten",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "Tarjoa p��valikon ohitusta",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10300
		  "���������� ������� ������� �������� ����",
		  "",
		  "",
		  "",
#if VDRVERSNUM >= 10342
		  "",
#endif
#endif
		},
		{ NULL },
	};

}
#endif