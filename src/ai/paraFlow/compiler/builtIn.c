
char *fetchBuiltInCode()
/* Return a string with the built in stuff. */
{
return
"global string programName;\n"
"global array of string args;\n"
"global bit true=1, false=0;\n"
"\n"
"global flow punt(string message, string source=\"application\", int level=0);\n"
"global flow print(var v);\n"
"global to prin(var v);\n"
"global flow ugly(var v);\n"
"global flow warn(string message);\n"
"global to stackDump();\n"
"global to keyIn() into string s;\n"
"global to lineIn() into string s;\n"
"global to random() into double zeroToOne;\n"
"global to randInit();\n"
"global flow sqrt(double x) into double y;\n"
"global flow atoi(string a) into int i;\n"
"global flow isSpace(char c) into bit b;\n"
"global to getEnvArray() into array of string envArray;\n"
"global flow milliTicks() into long milliseconds;\n"
"global flow floatToString(double f, int digitsBeforeDecimal=0, \n"
"		int digitsAfterDecimal=2, bit scientificNotation=0) \n"
"into (string s);\n"
"global flow intToString(long l, int minWidth, bit zeroPad, bit commas) into (string s);\n"
"\n"
"global class file\n"
"    {\n"
"    string name;\n"
"    to close();\n"
"    to read(int count) into (string s);\n"
"    to readLine() into (string s);\n"
"    to readAll() into (string s);\n"
"    to write(string s);\n"
"    to flush();\n"
"    to writeNow(string s);\n"
"    to put(var v);\n"
"    to get(var justForType) into var x;\n"
"    to seek(long pos, bit fromEnd=0);\n"
"    to skip(long amount);\n"
"    to tell() into long pos;\n"
"    }\n"
"\n"
"global flow fileOpen(string name, string mode) into file f;\n"
"global flow fileReadAll(string name) into (string s);\n"
"global flow fileExists(string name) into bit exists;\n"
"global to fileRename(string oldName, string newName);\n"
"global to fileRemove(string name);\n"
"global to httpConnect(string url, string method='GET', string agent='ParaFlow',"
	"string protocol='HTTP/1.0') into (file f);\n"
;
}

char *fetchStringDef()
/* Return a string with definition of string. */
{
return
"class _pf_string\n"
"    {\n"
"    int size;\n"
"    flow same(string other) into bit sameExceptForCase;\n"
"    flow first(int size=1) into string start;\n"
"    flow rest(int start=1) into string rest;\n"
"    flow middle(int start, int size) into string part;\n"
"    flow between(string start,end) into (string between);\n"
"    flow nextBetween(string start,end, int pos)\n"
"         into (string between, int nextPos);\n"
"    flow last(int size=1) into string end;\n"
"    flow upper() into (string uppered);\n"
"    flow lower() into (string lowered);\n"
"    flow find(string s) into (int foundPos);\n"
"    flow findNext(string s, int startPos) into int foundPos;\n"
"    flow findLast(string s) into (int foundPos);\n"
"    flow words() into (array of string words);\n"
"    flow lines() into (array of string lines);\n"	//!
"    flow tokens() into (array of string tokens);\n"
"    flow split(string separator) into (array of string split);\n" //!
"    flow nextWord(int pos) into (string s, int newPos);\n"
"    flow nextLine(int pos) into (string s, int newPos);\n"
"    flow nextToken(int pos) into (string s, int newPos);\n"
"    flow betweenQuotes(int pos) into (string s, int newPos);\n"
"    flow trim() into (string s);\n"
"    flow leadingSpaces(int pos=0) into (int count);\n"
"    flow fitLeft(int size) into (string s);\n"
"    flow fitRight(int size) into (string s);\n"
"    flow startsWith(string prefix) into (bit bool);\n"
"    flow endsWith(string suffix) into (bit bool);\n"
"    flow cgiEncode() into (string encoded);\n"
"    flow cgiDecode() into (string decoded);\n"
"    flow asInt() into (int x);\n"
"    flow asLong() into (long x);\n"
"    flow asDouble() into (double x);\n"
"    }\n"
"class _pf_dyString extends _pf_string\n"
"    {\n"
"    to append(string s);\n"
"    flow dupe() into string dupe;\n"
"    }\n"
"class _pf_elType\n"
"    {\n"
"    }\n"
"class _pf_array\n"
"    {\n"
"    int size;\n"
"    _operator_ append(_pf_elType el);\n"
"    to sort(var of flow (_pf_elType a, _pf_elType b) into (int v) cmp = nil);\n"
"    to push(_pf_elType el);\n"
"    to pop() into (_pf_elType el);\n"
"    }\n"
"class _pf_dir\n"
"    {\n"
"    to keys() into array of string keys;\n"
"    }\n"
;
}
