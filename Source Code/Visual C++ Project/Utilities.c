#include "4DPluginAPI.h"
#include "4DPlugin.h"

#include "PrivateTypes.h"
#include "EntryPoints.h"
#include "BaseTsd.h" // REB 3/30/11 #25290

void freeTextParameter(char *textParam)
{
	if (textParam != NULL) {
		free(textParam);
		textParam = NULL;
	}
}

int compareFileTimeCreation(WIN32_FIND_DATA* p1, WIN32_FIND_DATA* p2)
{
	int temp;
	FILETIME a;
	FILETIME b;

	a = p1->ftCreationTime;
	b = p2->ftCreationTime;
	temp = (int)CompareFileTime(&a, &b);

	return temp;
}

// WJF 4/7/15 #41624
int compareAlphabetical(WIN32_FIND_DATA* p1, WIN32_FIND_DATA* p2)
{
	int ret;
	char a[260];
	char b[260];

	strcpy(a, p1->cFileName);
	strcpy(a, p2->cFileName);

	ret = strcmp(a, b);

	return ret;
}

/* WJF 6/25/15 #42792 This function is not used anywhere
char * concatStr(const char *str1, const char *str2)
{
char *newStr = NULL;
INT_PTR newStrlen = 0;

newStrlen = strlen(str1) + strlen(str2) + 1;
newStr = (char *)malloc(newStrlen);

strcat(newStr, str1);
strcat(newStr, str2);

return newStr;
}*/

// MJG 7/5/05 #8372
// WJF 6/30/16 Win-21 LONG_PTR -> LONG
void setError(LONG value)
{
	PA_Variable   errorVar;

	errorVar = PA_GetVariable((PA_Unichar *) "Error"); // WJF 6/24/16 Win-21 char * -> PA_Unichar *

	if (PA_GetLastError() == eER_NoErr)
	{
		switch (PA_GetVariableKind(errorVar))
		{
		case eVK_Real:
			PA_SetRealVariable(&errorVar, (double)value);
			break;

		case eVK_Longint:
			PA_SetLongintVariable(&errorVar, value);
			break;
		}

		PA_SetVariable((PA_Unichar *) "Error", errorVar, 1); // WJF 6/24/16 Win-21 char * -> PA_Unichar *
	}
}

// REB 3/22/11 #25290 For ease of compatibility with the new unicode API, this method will
// function like the old PA_GetTextParameter.
// WJF 6/30/16 Win-21 LONG_PTR -> LONG
LONG PA_GetTextParameter(PA_PluginParameters params, short index, char* text)
{
	PA_Unistring			*UnistringText;
	LONG					length; // WJF 6/30/16 Win-21 LONG_PTR -> LONG
	char					*textParameter;

	UnistringText = PA_GetStringParameter(params, index);

	if (text != 0L){
		textParameter = malloc((UnistringText->fLength + 1) * sizeof(char));
		memset(textParameter, 0, ((UnistringText->fLength + 1) * sizeof(char)));

		wcstombs(textParameter, UnistringText->fString, UnistringText->fLength);
		textParameter[strlen(textParameter)] = '\0';

		strcpy(text, textParameter);

		free(textParameter);

		length = (LONG)strlen(text); // WJF 6/30/16 Win-21 Cast to LONG
	}
	else{
		length = UnistringText->fLength;
	}

	return length;
}

// REB 3/22/11 #25290 For ease of compatibility with the new unicode API, this method will
// function like the old PA_SetTextParameter.
void PA_SetTextParameter(PA_PluginParameters params, short index, char* text, LONG_PTR len)
{
	PA_Unistring		*UnistringParam;
	PA_Unichar			*translatedText;
	LONG_PTR			length;

	if (len > 0){
		length = mbstowcs(0, text, len) + 1;

		translatedText = malloc(length * sizeof(PA_Unichar));

		if (translatedText != NULL){
			memset(translatedText, 0, (length * sizeof(PA_Unichar)));

			mbstowcs(translatedText, text, len);

			UnistringParam = PA_GetStringParameter(params, index);

			PA_SetUnistring(UnistringParam, translatedText);
		}

		free(translatedText); // WJF 6/4/15 #42921
	}
}

// REB 3/28/11 #25290 For ease of compatibility with the new unicode API, this method will
// function like the old PA_SetTextInArray.
// WJF 6/30/16 Win-21 LONG_PTR -> LONG
void PA_SetTextInArray(PA_Variable array4D, LONG index, char* text, LONG_PTR len)
{
	PA_Unistring		UnistringText;
	PA_Unichar			*translatedText = NULL;
	LONG_PTR			length = 0;

	length = mbstowcs(0, text, len) + 1;

	translatedText = malloc(length * sizeof(PA_Unichar));

	if (translatedText != NULL){
		memset(translatedText, 0, (length * sizeof(PA_Unichar)));

		mbstowcs(translatedText, text, len);

		UnistringText = PA_CreateUnistring(translatedText);

		PA_SetStringInArray(array4D, index, &UnistringText);

		// free(translatedText); // WJF 6/4/15 #42792 // WJF 9/25/15 #43733 Removed cause this could cause a crash
	}
}

// REB 3/28/11 #25290 For ease of compatibility with the new unicode API, this method will
// function like the old PA_GetTextInArray.
// WJF 6/29/16 Win-21 LONG_PTR -> LONG
LONG PA_GetTextInArray(PA_Variable array4D, LONG index, char* text)
{
	PA_Unistring		UnistringText;
	char				*translatedText;
	LONG				length = 0; // WJF 6/30/16 Win-21 LONG_PTR -> LONG

	UnistringText = PA_GetStringInArray(array4D, index);

	if (text != 0L){
		translatedText = malloc((UnistringText.fLength + 1)* sizeof(char));

		if (translatedText != NULL){
			memset(translatedText, 0, ((UnistringText.fLength + 1)* sizeof(char)));

			wcstombs(translatedText, UnistringText.fString, UnistringText.fLength);

			translatedText[strlen(translatedText)] = '\0';

			strcpy(text, translatedText);

			free(translatedText);

			length = (LONG)strlen(text); // WJF 6/30/16 Win-21 Cast to LONG
		}
	}
	else{
		length = UnistringText.fLength;
	}

	return length;
}

// REB 3/28/11 #25290 For ease of compatibility with the new unicode API, this method will
// function like the old PA_ReturnText.
void PA_ReturnText(PA_PluginParameters params, char* text, LONG_PTR len){
	PA_Unichar			*translatedText;
	LONG_PTR			length;

	length = mbstowcs(0, text, len) + 1;

	translatedText = malloc(length * sizeof(PA_Unichar));

	if (translatedText != NULL){
		memset(translatedText, 0, (length * sizeof(PA_Unichar)));

		mbstowcs(translatedText, text, len);

		PA_ReturnString(params, translatedText);

		free(translatedText); // WJF 5/20/15 #42772
	}
}

// WJF 6/25/15 #42792 When using this function, be sure to free the returned character string.
char * getTextParameter(PA_PluginParameters param, short index)
{
	char *textValue = NULL;
	LONG_PTR textLength = 0;

	textLength = PA_GetTextParameter(param, index, 0L) + 1;

	// modified by Mark de Wever on 20060721
	// textLength was always > 0 due to the +1
	// revert change since sys_ShellExecute
	// fails if NULL pointer is returned...
	// So if malloc fails it will crash 4D
	// if(textLength > 1) {
	if (textLength > 0) {
		textValue = malloc(textLength); // WJF 7/13/16 Win-21 Removed typecasting on malloc to follow C best practices

		if (textValue != NULL) {
			memset(textValue, 0, textLength);
			textLength = PA_GetTextParameter(param, index, textValue);
		}
	}

	return textValue;
}

// REB 4/20/11 #27322 Convert a C string to a 4D Unistring
PA_Unistring CStringToUnistring(char* text)
{
	PA_Unistring		UnistringText;
	PA_Unichar			*translatedText = NULL;
	LONG_PTR			length = 0;

	length = mbstowcs(0, text, strlen(text)) + 1;

	translatedText = malloc(length * sizeof(PA_Unichar));

	if (translatedText != NULL){
		memset(translatedText, 0, (length * sizeof(PA_Unichar)));

		mbstowcs(translatedText, text, strlen(text));

		UnistringText = PA_CreateUnistring(translatedText);

		free(translatedText); // WJF 6/25/15 #42792

		return UnistringText;
	}
	else { // WJF 6/24/16 Win-21
		UnistringText = PA_CreateUnistring(NULL);
		return UnistringText;
	}
}

// REB 4/20/11 #27322 Convert a 4D Unistring to a C string
char* UnistringToCString(PA_Unistring* UnistringText)
{
	char* translatedText;
	const char * strError = "Error allocating memory of string.";
	size_t errLength = strlen(strError);

	translatedText = malloc((UnistringText->fLength + 1)* sizeof(char));

	if (translatedText != NULL){
		memset(translatedText, 0, ((UnistringText->fLength + 1)* sizeof(char)));

		wcstombs(translatedText, UnistringText->fString, UnistringText->fLength);

		translatedText[strlen(translatedText)] = '\0';

		return translatedText;
	}
	else {
		translatedText = malloc(errLength); // WJF 7/13/16 Win-21 Removed typecasting on malloc to follow C best practices
		strcpy_s(translatedText, errLength, strError);
		return translatedText;
	}
}

/*
char* replace_str(char *str, char *orig, char *rep)
{
static char buffer[4096];
LONG_PTR i = 0;

char *p;

p = orig;

//First count the number of characters we need to replace.
while(p != "/0"){
if((char)p == (char)rep){
i++;
}
}

if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
return str;

strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
buffer[p-str] = '\0';

sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

return buffer;
}

char* replace_str(char *str, char *orig, char *rep)
{
static char buffer[4096];
char *p;

if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
return str;

strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
buffer[p-str] = '\0';

sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

return buffer;
}

*/