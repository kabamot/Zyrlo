#ifndef _ZYRLO_OCR_H_
#define _ZYRLO_OCR_H_

#include <opencv2/opencv.hpp>

#define MAX_LINE_LENGTH 2048

#define ZRL_ENGLISH_US		0x1ULL
#define ZRL_FRENCH			0x2ULL
#define ZRL_GERMAN			0x4ULL
#define ZRL_ITALIAN			0x8ULL
#define ZRL_SPANISH			0x10ULL
#define ZRL_PORTUGUESE		0x20ULL
#define ZRL_DANISH			0x40ULL
#define ZRL_DUTCH 			0x80ULL
#define ZRL_NORWEGIAN		0x100ULL
#define ZRL_SWEDISH			0x200ULL
#define ZRL_RUSSIAN			0x400ULL
#define ZRL_FINNISH			0x800ULL
#define ZRL_SLOVENIAN		0x1000ULL
#define ZRL_POLISH			0x2000ULL
#define ZRL_ENGLISH_UK		0x4000ULL
#define ZRL_SPANISH_MX		0x8000ULL
#define ZRL_ENGLISH_IN		0x10000ULL
#define ZRL_HEBREW			0x20000ULL
#define ZRL_WELSH			0x40000ULL
#define ZRL_BELGIAN_DUTCH 	0x80000ULL
#define ZRL_ENGLISH_AU		0x100000ULL
#define ZRL_FRENCH_CA		0x200000ULL
#define ZRL_TURKISH			0x400000ULL
#define ZRL_HUNGARIAN		0x800000ULL
#define ZRL_CZECH			0x1000000ULL
#define ZRL_SERB_CYR		0x2000000ULL
#define ZRL_ICELANDIC		0x4000000ULL
#define ZRL_AFRIKAANS		0x8000000ULL
#define ZRL_PORTUGUESE_BR	0x10000000ULL
#define ZRL_CHINESE			0x20000000ULL
#define ZRL_GREEK			0x40000000ULL
#define ZRL_KOREAN			0x80000000ULL
#define ZRL_ARABIC			0x100000000ULL
#define ZRL_SLOVAK			0x200000000ULL

inline unsigned long long langToMask(const char * lang) {
    if(strcmp(lang, "eng") == 0)
        return ZRL_ENGLISH_US;
    if(strcmp(lang, "fre") == 0)
        return ZRL_FRENCH;
    if(strcmp(lang, "ger") == 0)
        return ZRL_GERMAN;
    if(strcmp(lang, "ita") == 0)
        return ZRL_ITALIAN;
    if(strcmp(lang, "spa") == 0)
        return ZRL_SPANISH;
    if(strcmp(lang, "por") == 0)
        return ZRL_PORTUGUESE;
    if(strcmp(lang, "dan") == 0)
        return ZRL_DANISH;
    if(strcmp(lang, "dut") == 0)
        return ZRL_DUTCH;
    if(strcmp(lang, "nor") == 0)
        return ZRL_NORWEGIAN;
    if(strcmp(lang, "rus") == 0)
        return ZRL_RUSSIAN;
    if(strcmp(lang, "swe") == 0)
        return ZRL_SWEDISH;
    if(strcmp(lang, "fin") == 0)
        return ZRL_FINNISH;
    if(strcmp(lang, "slv") == 0)
        return ZRL_SLOVENIAN;
    if(strcmp(lang, "pol") == 0)
        return ZRL_POLISH;
    if(strcmp(lang, "heb") == 0)
        return ZRL_HEBREW;
    if(strcmp(lang, "wel") == 0)
        return ZRL_WELSH;
    if(strcmp(lang, "tur") == 0)
        return ZRL_TURKISH;
    if(strcmp(lang, "hun") == 0)
        return ZRL_HUNGARIAN;
    if(strcmp(lang, "ces") == 0)
        return ZRL_CZECH;
    if(strcmp(lang, "srp") == 0)
        return ZRL_SERB_CYR;
    if(strcmp(lang, "ice") == 0)
        return ZRL_ICELANDIC;
    if(strcmp(lang, "afr") == 0)
        return ZRL_AFRIKAANS;
    if(strcmp(lang, "chn") == 0)
        return ZRL_CHINESE;
    if(strcmp(lang, "ell") == 0)
        return ZRL_GREEK;
    if(strcmp(lang, "kor") == 0)
        return ZRL_KOREAN;
    if(strcmp(lang, "ara") == 0)
        return ZRL_ARABIC;
    if(strcmp(lang, "slk") == 0)
        return ZRL_SLOVAK;
      return 0;
 }

extern "C" {

typedef struct _text_line {
    int nLineId;    // Line ID is unique for the page
    int nParagraphId;
    char sLang[16], sText[MAX_LINE_LENGTH];
    bool isHeader;
    bool isNewLine;
} text_line;

// Initializer the library. Call after boot.
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_init(const char *sDataDir, unsigned long long nLanguage);

// Start processing image.
// Partams: sImgPath - any image (jpg, bmp, etc)
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_start(const char *sImgPath);

// Start processing image.
// Params: bayer OpenCV image in raw bayer format.
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_start_with_bayer(const cv::Mat &bayer);

// Start processing image.
// Params: bayer OpenCV image in grey format.
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_start_img(const cv::Mat &img);

// Clean up before shutdown/restart
void zyrlo_proc_end();

// Return result of one line in XML format.
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_get_results_xml(char *sResult, int nMaxSize);
//
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_get_result(text_line *pTextLine);

// return # of paragraphs
// returns -1 if error or unknown
int zyrlo_proc_get_num_paragraphs();

// return # of lines in a paragraph
// return -1 if error, unknown or paragraph does not exist
int zyrlo_proc_get_num_lines(int nParagraph);

// get current status
// Idle
// Initializing
// Pre-processing
// Processing
// Zoning
// Init OCRing  (looking for orientation)
// Ocring	(orientation determmined)
// Cancelling
int zyrlo_proc_get_status(char *sStatus);

// Cancel processing. Should be called before starting new image if status is not Idle
int zyrlo_proc_cancel();

// Set OCR language. For now set only one language.
// Parameter:	nLanguage - language ID defined in Languages.h
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_set_ocr_language(unsigned long long nLanguage);

// Get text of a whole page
// caller should call delete() to free memory
char* zyrlo_proc_get_text();

// Jump to nParagraph even if current paragraph OCR is not complete
// return 	0 if OK and line is returned
//		<0 if error
int zurlo_proc_switch_to_paragraph(int nParagraph);

int zurlo_proc_set_force_single_column(bool bForceSingleColumn);
int zurlo_proc_get_force_single_column(bool *bForceSingleColumn);

}

#endif //_ZYRLO_OCR_H_
