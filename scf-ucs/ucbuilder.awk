BEGIN {
	FS = ";"
    print("/* Warning - this file was built by a tool */\n");
    print("#include \"ucsdb.h\"\n")
    print("static ucs_details unicode_data[] = {")
	first = 1
}

{
	if (first == 1) {
		leading = ""
		first = 0
	} else {
		leading = ","
	}

	cp = "0x" $1
	upper_cp = cp
	lower_cp = cp
	title_cp = cp
	
	if ($13 != "") upper_cp = "0x" $13
	if ($14 != "") lower_cp = "0x" $14
	if ($15 != "") title_cp = "0x" $15
	
	digit_value = "-1"
	if ($8 != "") {
		digit_value = $8
	}
	
	cat = "UCS_OTHER"
	if ($3 == "Ll") cat = "UCS_LETTER | UCS_LOWER"
	if ($3 == "Lu") cat = "UCS_LETTER | UCS_UPPER"
	if ($3 == "Lt") cat = "UCS_LETTER | UCS_TITLE"
	if ($3 == "Lo") cat = "UCS_LETTER"
	if ($3 == "Nd") cat = "UCS_DIGIT"
	if ($1 == "0009" || $1 == "0020") cat = "UCS_SPACE"
	
	if ($3 != "Cs" && $3 != "Co") {
		printf("%s{%s, %s, %s, %s, %s, %s}\n", leading, cp, cat, digit_value, upper_cp, lower_cp, title_cp)
	}
}

END {
	print "};"
}
