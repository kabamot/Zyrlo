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

extern "C" {

typedef struct _text_line {
    int nLineId;    // Line ID is unique for the page
    int nParagraphId;
    char sText[MAX_LINE_LENGTH];
    bool isHeader;
    bool isNewLine;
} text_line;

// Initializer the library. Call after boot.
// return 	0 if OK and line is returned
//		<0 if error
int zyrlo_proc_init(const char *sDataDir, int nLanguage);

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
int zyrlo_proc_set_ocr_language(int nLanguage);

// Get text of a whole page
// caller should call delete() to free memory
char* zyrlo_proc_get_text();

// Jump to nParagraph even if current paragraph OCR is not complete
// return 	0 if OK and line is returned
//		<0 if error
int zurlo_proc_switch_to_paragraph(int nParagraph);

}

#endif //_ZYRLO_OCR_H_
