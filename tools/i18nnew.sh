#!/bin/bash

# based on the Makagiga's tools/i18n.sh script

if [ -z "${1}" ]; then
	echo "Description:           Creates a new translation file (po/LANG_CODE.po)"
	echo "                       from the template file (po/TEMPLATE.pot)"
	echo "Usage:                 ./tools/i18nnew.sh LANG_CODE"
	echo "Example (German i18n): ./tools/i18nnew.sh de"
	echo "Output file:           ./po/de.po"
	echo
	echo "ERROR: Missing LANG_CODE parameter. See Example above."
	exit 1
fi

OUT_FILE="po/${1}.po"

if [ -f "${OUT_FILE}" ]; then
	echo "${0}: File already exists: \"${OUT_FILE}\""
	exit 1
fi

msginit --input="po/TEMPLATE.pot" --output-file="${OUT_FILE}"

echo
echo "**** NOTE: USE UTF-8 CHARSET IN THE \"${OUT_FILE}\" FILE ****"
echo
