#!/bin/bash

if [ ! -f "./tools/i18n.sh" ]; then
	echo "Usage: ./tools/`basename $0`"
	exit 1
fi

# based on the Makagiga's tools/i18n.sh script

set -e

mkdir -p po

# remove generated "object" files
#rm -f po/*.mo
rm -f src/i18n/*.qm

# create list of all *.cpp and *.h files
find src -name "*.cpp" -or -name "*.h"|sort>po/list.tmp

# create translation template
xgettext \
	--files-from=po/list.tmp \
	--force-po \
	--keyword=i18n \
	--keyword=ki18n \
	--output=po/TEMPLATE.pot

# remove backups older than one week
find po -name "*.po*~" -mtime "+7" -delete

# merge "po/*.po"
for i in po/*.po; do
	if [ -f $i ]; then
		# create backup file
		cp "$i" "$i.`date "+%Y%m%d_%H%M_%S"`~"
		
		echo
		echo "==== Creating $i translation ===="
		msgmerge "$i" po/TEMPLATE.pot --output-file="$i"
		
#		echo "Creating KDE messages..."
#		msgfmt \
#			"$i" \
#			--output-file="po/$(basename "$i" .po).mo" \
#			--statistics

		echo "Creating Qt messages..."
		msgfmt \
			"$i" \
			--output-file="src/i18n/kshutdown_$(basename "$i" .po).qm" \
			--qt \
			--statistics

		if ! POFileChecker --ignore-fuzzy "$i"; then
			echo "NOTE: POFileChecker (optional gettext-lint package) not installed"
		fi
	fi
done

# clean up
rm po/list.tmp