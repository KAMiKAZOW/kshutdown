# Based on the KEuroCalc - A currency converter and calculator
# http://opensource.bureau-cornavin.com/keurocalc/index.html

FILE(GLOB mo_files *.mo)
FOREACH(file ${mo_files})
	GET_FILENAME_COMPONENT(lang ${file} NAME_WE)
	GET_FILENAME_COMPONENT(out ${lang}.mo ABSOLUTE)
	INSTALL(
		FILES ${out}
		DESTINATION ${LOCALE_INSTALL_DIR}/${lang}/LC_MESSAGES/
		RENAME kshutdown.mo
	)
ENDFOREACH(file ${mo_files})