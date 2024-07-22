#include "cvDecodeClass.hpp"
#include "cvDecodeWrapper.h"

extern "C" {

	CvDecode *newCvDecode() {
		return new CvDecode();
	}

	void CvDecode_decode(CvDecode *v, char *p_buffer, int i_buffer) {

		v -> cvDecode(p_buffer, i_buffer);

	}

	void deleteCvDecode(CvDecode *v) {
		delete v;
	}

}
