function utf8_byte(cp, prefix, shift, mask, tmp) {
	tmp = int(cp / shift)
	return prefix + (tmp % mask)
}

function cp_to_utf8(cp, b1, b2, b3, b4) {
	b2 = 0
	b3 = 0
	b4 = 0
	cp = "0x" cp
	cp = cp + 0
	if (cp <= 127) {
		b1 = cp;
	} else if (cp <= 2047) {
		b1 = utf8_byte(cp, 128, 1, 64)
		b2 = utf8_byte(cp, 192, 64, 256)
	} else if (cp <= 65535) {
		b1 = utf8_byte(cp, 128, 1, 64)
		b2 = utf8_byte(cp, 128, 64, 64)
		b3 = utf8_byte(cp, 224, 64 * 64, 256)
	} else {
		b1 = utf8_byte(cp, 128, 1, 64)
		b2 = utf8_byte(cp, 128, 64, 64)
		b3 = utf8_byte(cp, 128, 64 * 64, 64)
		b4 = utf8_byte(cp, 240, 64 * 64 * 64, 256)
	}
	
	return b1 + (b2 * 256) + (b3 * 256 * 256) + (b4 * 256 * 256 * 256)
}

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
	utf8 = cp_to_utf8($1)
	upper_cp = utf8
	lower_cp = utf8
	title_cp = utf8
	
	if ($13 != "") upper_cp = cp_to_utf8($13)
	if ($14 != "") lower_cp = cp_to_utf8($14)
	if ($15 != "") title_cp = cp_to_utf8($15)
	
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
		printf("%s{0x%x, %s, %s, %s, 0x%x, 0x%x, 0x%x}\n", leading, utf8, cp, cat, digit_value, upper_cp, lower_cp, title_cp)
	}
}

END {
	print "};"
}
