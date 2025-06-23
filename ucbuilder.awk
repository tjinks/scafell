BEGIN {
	FS = ";"
    print("/* Warning - this file was built by a tool */\n");
    print("#include \"ucdb.h\"\n")
    print("static struct {");
    print("    int codepoint;");
    print("    scf_char_category category;");
    print("    int digit_value;");
    print("    int uc_codepoint;");
    print("    int lc_codepoint;");
    print("    int tc_codepoint;");
	print("} uc_database[] = {")
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
	
	cat = "UC_OTHER"
	if ($3 == "Ll") cat = "UC_LETTER | UC_LOWER"
	if ($3 == "Lu") cat = "UC_LETTER | UC_UPPER"
	if ($3 == "Lt") cat = "UC_LETTER | UC_TITLE"
	if ($3 == "Lo") cat = "UC_LETTER"
	if ($3 == "Nd") cat = "UC_DIGIT"
	if ($1 == "0009" || $1 == "0020") cat = "UC_SPACE"
		
	printf("%s{%s, %s, %s, %s, %s, %s}\n", leading, cp, cat, digit_value, upper_cp, lower_cp, title_cp)
}

END {
	print "};"
}
