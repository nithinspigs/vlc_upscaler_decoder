#ifdef __cplusplus
extern "C" {
#endif

typedef struct CvDecode CvDecode;

CvDecode *newCvDecode();

void CvDecode_decode(CvDecode *v, unsigned char *p_buffer, int i_buffer);

void deleteCvDecode(CvDecode *v);

#ifdef __cplusplus
}
#endif
