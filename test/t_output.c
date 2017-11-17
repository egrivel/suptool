#include "unittest.h"
#include "../common.h"
#include "../output.h"
#include "../charlist.h"

#define TEST_FILENAME "text.txt"
#define TEST_WIDTH    720
#define TEST_HEIGHT   480

void test_string_with_style(char *in1, int style1, char *in2, int style2,
                            char *result) {
   char *fmt = "Check string '%s%s' --> '%s'";
   char *buffer = malloc(strlen(in1) + strlen(in2) + strlen(result) + strlen(fmt));
   int i;

   output_open(TEST_FILENAME, FORMAT_SRT, TEST_WIDTH, TEST_HEIGHT);
   if ((in1 != NULL) && strlen(in1)) {
      for (i = 0; i < strlen(in1); i++) {
         int j = 0;
         buffer[j++] = in1[i];
         if (in1[i] == '\\') {
            buffer[j++] = in1[++i];
         }
         buffer[j++] = '\0';
         output_string(buffer, style1);
      }
   }
   if ((in2 != NULL) && strlen(in2)) {
      for (i = 0; i < strlen(in2); i++) {
         int j = 0;
         buffer[j++] = in2[i];
         if (in2[i] == '\\') {
            buffer[j++] = in2[++i];
         }
         buffer[j++] = '\0';
         output_string(buffer, style2);
      }
   }
   output_close();
   sprintf(buffer, fmt, in1, in2, result);
   unit_step(buffer);
   unit_file_content(TEST_FILENAME, result);
   unit_file_remove(TEST_FILENAME);
   free(buffer);
}

void test_string(char *in, char *result) {
   test_string_with_style(in, STYLE_NORMAL, "", STYLE_NORMAL, result);
}

void test_string_normal_italic(char *in1, char *in2, char *result) {
   test_string_with_style(in1, STYLE_NORMAL, in2, STYLE_ITALIC, result);
}

void test_string_italic_normal(char *in1, char *in2, char *result) {
   test_string_with_style(in1, STYLE_ITALIC, in2, STYLE_NORMAL, result);
}


void test1() {
   unit_start("1. Create output file");
   unit_step("File does not exist before test");
   unit_file_not_exists(TEST_FILENAME);
   unit_step("File exists after open");
   output_open(TEST_FILENAME, FORMAT_SRT, TEST_WIDTH, TEST_HEIGHT);
   unit_file_exists(TEST_FILENAME);
   unit_step("File still exists after close");
   output_close();
   unit_file_exists(TEST_FILENAME);
   unit_step("Remove output file");
   unit_file_remove(TEST_FILENAME);
   unit_file_not_exists(TEST_FILENAME);
   unit_done();
}

void test2() {
   unit_start("2. Write text to output file");
   unit_step("Open file");
   output_open(TEST_FILENAME, FORMAT_SRT, TEST_WIDTH, TEST_HEIGHT);
   unit_file_exists(TEST_FILENAME);
   unit_step("Write to test file");
   output_string("Simple test string", STYLE_NORMAL);
   output_newline();
   unit_step("Close file");
   output_close();
   unit_step("Check file content");
   unit_file_content(TEST_FILENAME, "Simple test string\n");
   unit_step("Remove output file");
   unit_file_remove(TEST_FILENAME);
   unit_file_not_exists(TEST_FILENAME);
   unit_done();
}

void test3() {
   unit_start("3. Write multiple lines of text to output file");
   unit_step("Open file");
   output_open(TEST_FILENAME, FORMAT_SRT, TEST_WIDTH, TEST_HEIGHT);
   unit_file_exists(TEST_FILENAME);
   unit_step("Write to test file");
   output_string("Test string1", STYLE_NORMAL);
   output_string("Test string2", STYLE_NORMAL);
   output_newline();
   output_string("Test string3", STYLE_NORMAL);
   output_newline();
   unit_step("Close file");
   output_close();
   unit_step("Check file content");
   unit_file_content(TEST_FILENAME, "Test string1Test string2\nTest string3\n");
   unit_step("Remove output file");
   unit_file_remove(TEST_FILENAME);
   unit_file_not_exists(TEST_FILENAME);
   unit_done();
}

void test4() {
   unit_start("4. Check fixing of '\\I' to 'l' or 'I'");
   // any \I followed by a capital letter is I
   test_string("S\\IMON", "SIMON");

   // a stand-alone \I is I
   test_string("\\I text", "I text");
   test_string("text \\I text", "text I text");
   test_string("text \\I", "text I");

   // stand-alone also means followed by punctuation
   test_string("text \\I' text", "text I' text");
   test_string("text \\I\" text", "text I\" text");
   test_string("text \\I. text", "text I. text");
   test_string("text \\I, text", "text I, text");
   test_string("text \\I? text", "text I? text");
   test_string("text \\I! text", "text I! text");
   test_string("text \\I: text", "text I: text");
   test_string("text \\I; text", "text I; text");
   test_string("text \\I- text", "text I- text");

   // double \I\I should be ll unless roman numeral
   test_string("A\\I\\Iegations", "Allegations");
   test_string("text \\I\\I text", "text II text");
   test_string("text V\\I\\I text", "text VII text");
   test_string("text X\\I\\I text", "text XII text");
   test_string("text L\\I\\I text", "text LII text");
   test_string("text C\\I\\I text", "text CII text");
   test_string("text D\\I\\I text", "text DII text");
   test_string("text M\\I\\I text", "text MII text");

   // \I inside a word (not first letter) should be l, unless all
   // caps
   test_string("a\\Ibc", "albc");
   test_string("A\\Ibc", "Albc");
   test_string("A\\IBC", "AIBC");

   // word starting with \I following a word should be l
   test_string("A \\Iist", "A list");
   test_string("I \\Iike", "I like");
   test_string("is a \\Iist", "is a list");
   test_string("what I \\Iike", "what I like");

   // Start of the line, starts with capital
   test_string("\\In", "In");
   test_string("\"\\In", "\"In");
   test_string("'\\In", "'In");
   test_string("-\\In", "-In");
   // but with a space in front, it does not
   test_string(" \\Iate", " late");
   test_string(" \"\\Iate", " \"late");
   test_string(" '\\Iate", " 'late");
   test_string(" -\\Iate", " -late");

   // Miscellaneous tests
   test_string("THE \\INDOOR", "THE INDOOR");
   test_string("\\IV text", "IV text");
   test_string("V\\I text", "VI text");
   test_string("V\\Iadimir", "Vladimir");
   unit_done();
}
   
void test5() {
   unit_start("5. Check fixing of '\\I' to 'l' or 'I', changing normal to italic");
   // a stand-alone \I is I
   test_string_normal_italic("\\I", " text", "I <i>text</i>");
   test_string_normal_italic("text ", "\\I text", "text <i>I text</i>");
   test_string_normal_italic("text ", "\\I", "text <i>I</i>");

   // stand-alone also means followed by punctuation
   test_string_normal_italic("text ", "\\I' text", "text <i>I' text</i>");
   test_string_normal_italic("text ", "\\I\" text", "text <i>I\" text</i>");
   test_string_normal_italic("text ", "\\I. text", "text <i>I. text</i>");
   test_string_normal_italic("text ", "\\I, text", "text <i>I, text</i>");
   test_string_normal_italic("text ", "\\I? text", "text <i>I? text</i>");
   test_string_normal_italic("text ", "\\I! text", "text <i>I! text</i>");
   test_string_normal_italic("text ", "\\I: text", "text <i>I: text</i>");
   test_string_normal_italic("text ", "\\I; text", "text <i>I; text</i>");
   test_string_normal_italic("text ", "\\I- text", "text <i>I- text</i>");

   // double \I\I should be ll unless roman numeral
   test_string_normal_italic("text ", "\\I\\I text", "text <i>II text</i>");
   test_string_normal_italic("text ", "V\\I\\I text", "text <i>VII text</i>");
   test_string_normal_italic("text ", "X\\I\\I text", "text <i>XII text</i>");
   test_string_normal_italic("text ", "L\\I\\I text", "text <i>LII text</i>");
   test_string_normal_italic("text ", "C\\I\\I text", "text <i>CII text</i>");
   test_string_normal_italic("text ", "D\\I\\I text", "text <i>DII text</i>");
   test_string_normal_italic("text ", "M\\I\\I text", "text <i>MII text</i>");

   // word starting with \I following a word should be l
   test_string_normal_italic("A ", "\\Iist", "A <i>list</i>");
   test_string_normal_italic("I ", "\\Iike", "I <i>like</i>");
   test_string_normal_italic("\\I ", "\\Iike", "I <i>like</i>");
   test_string_normal_italic("is a ", "\\Iist", "is a <i>list</i>");
   test_string_normal_italic("what I ", "\\Iike", "what I <i>like</i>");
   test_string_normal_italic("what \\I ", "\\Iike", "what I <i>like</i>");

   // Start of the line, starts with capital
   test_string_normal_italic("\"", "\\In", "\"<i>In</i>");
   test_string_normal_italic("'", "\\In", "'<i>In</i>");
   test_string_normal_italic("-", "\\In", "-<i>In</i>");
   // but with a space in front, it does not
   test_string_normal_italic(" ", "\\Iate", " <i>late</i>");
   test_string_normal_italic(" \"", "\\Iate", " \"<i>late</i>");
   test_string_normal_italic(" '", "\\Iate", " '<i>late</i>");
   test_string_normal_italic(" -", "\\Iate", " -<i>late</i>");

   // Miscellaneous tests
   test_string_normal_italic("THE ", "\\INDOOR", "THE <i>INDOOR</i>");
   unit_done();
}
   
void test6() {
   unit_start("6. Check fixing of '\\I' to 'l' or 'I', changing italic to normal");
   // any \I followed by a capital letter is I
   test_string_italic_normal("S", "\\IMON", "<i>S</i>IMON");

   // a stand-alone \I is I
   test_string_italic_normal("\\I", " text", "<i>I</i> text");
   test_string_italic_normal("text ", "\\I text", "<i>text</i> I text");
   test_string_italic_normal("text ", "\\I", "<i>text</i> I");

   // stand-alone also means followed by punctuation
   test_string_italic_normal("text ", "\\I' text", "<i>text</i> I' text");
   test_string_italic_normal("text ", "\\I\" text", "<i>text</i> I\" text");
   test_string_italic_normal("text ", "\\I. text", "<i>text</i> I. text");
   test_string_italic_normal("text ", "\\I, text", "<i>text</i> I, text");
   test_string_italic_normal("text ", "\\I? text", "<i>text</i> I? text");
   test_string_italic_normal("text ", "\\I! text", "<i>text</i> I! text");
   test_string_italic_normal("text ", "\\I: text", "<i>text</i> I: text");
   test_string_italic_normal("text ", "\\I; text", "<i>text</i> I; text");
   test_string_italic_normal("text ", "\\I- text", "<i>text</i> I- text");

   // double \I\I should be ll unless roman numeral
   test_string_italic_normal("text ", "\\I\\I text", "<i>text</i> II text");
   test_string_italic_normal("text ", "V\\I\\I text", "<i>text</i> VII text");
   test_string_italic_normal("text ", "X\\I\\I text", "<i>text</i> XII text");
   test_string_italic_normal("text ", "L\\I\\I text", "<i>text</i> LII text");
   test_string_italic_normal("text ", "C\\I\\I text", "<i>text</i> CII text");
   test_string_italic_normal("text ", "D\\I\\I text", "<i>text</i> DII text");
   test_string_italic_normal("text ", "M\\I\\I text", "<i>text</i> MII text");

   // word starting with \I following a word should be l
   test_string_italic_normal("A ", "\\Iist", "<i>A</i> list");
   test_string_italic_normal("\\I ", "\\Iike", "<i>I</i> like");
   test_string_italic_normal("is a ", "\\Iist", "<i>is a</i> list");
   test_string_italic_normal("what \\I ", "\\Iike", "<i>what I</i> like");

   // Start of the line, starts with capital
   test_string_italic_normal("\"", "\\In", "<i>\"</i>In");
   test_string_italic_normal("'", "\\In", "<i>'</i>In");
   test_string_italic_normal("-", "\\In", "<i>-</i>In");
   // but with a space in front, it does not
   test_string_italic_normal(" ", "\\Iate", " late");
   test_string_italic_normal(" \"", "\\Iate", " <i>\"</i>late");
   test_string_italic_normal(" '", "\\Iate", " <i>'</i>late");
   test_string_italic_normal(" -", "\\Iate", " <i>-</i>late");

   // Miscellaneous tests
   test_string_italic_normal("THE ", "\\INDOOR", "<i>THE</i> INDOOR");
   unit_done();
}
   
void test7() {
   unit_start("7. Check fixing of 'l' to 'I' in italic text");
   unit_step("Open file");
   output_open(TEST_FILENAME, FORMAT_SRT, TEST_WIDTH, TEST_HEIGHT);
   unit_file_exists(TEST_FILENAME);
   unit_step("Write to test file");
   output_string("A\\I\\Iegations", STYLE_ITALIC);
   unit_step("Close file");
   output_close();
   unit_step("Check file content");
   unit_file_content(TEST_FILENAME, "<i>Allegations</i>");
   unit_step("Remove output file");
   unit_file_remove(TEST_FILENAME);
   unit_file_not_exists(TEST_FILENAME);
   unit_done();
}

void test_x() {
   unit_start("x. Check fixing of '\\I' to 'l' or 'I', changing italic to normal");
   // but with a space in front, it does not
   test_string_italic_normal(" ", "\\Iate", " late");
   unit_done();
}
   

void test_it() {
   // test_x();
   test1();
   test2();
   test3();
   test4();
   test5();
   test6();
   test7();
}
