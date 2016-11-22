
#include <string.h>

#include "info.h"
#include "lm.h"
#include "s3types.h"
#include "cmd_ln.h"
#include "cmdln_macro.h"
#include "encoding.h"




int
lm_convert(char* inflie, char *outfile)
{
    const char *inputfn;
    const char *outputfn;
    char *local_outputfn;
    const char *inputfmt;
    const char *outputfmt;
    const char *inputenc;
    const char *outputenc;
    const char *outputdir;
    char *outputpath;
    int outputfnfree = FALSE;
    lm_t *lm;
    char separator[2];


    inputfn = NULL;
    outputfn = local_outputfn = NULL;
    inputfmt = NULL;
    outputfmt = NULL;
    outputdir = NULL;

    inputfn = inflie;
	outputfn = outfile;

    inputfmt = "TXT";
    outputfmt = "DMP";

    inputenc = "UTF-8";
    outputenc = "UTF-8";

    outputdir = ".";

    if (!strcmp(inputfmt, outputfmt) && !strcmp(inputenc, outputenc))
        E_FATAL
            ("Input and Output file formats and encodings are the same (%s, %s). Do nothing\n",
             inputfmt, inputenc);

    /* Read LM */
    if ((lm =
         lm_read_advance2(inputfn, "default", 1.0, 0.1, 1.0, 0, inputfmt,
                          0, 1, NULL)) == NULL)
        E_FATAL("Fail to read inputfn %s in inputfmt %s\n", inputfn,
                inputfmt);

    if (outputfn == NULL) {
      /* Length = strlen(inputfn) + 1 + strlen(outputfmt) + 5 (For safety) */
      outputfn = local_outputfn = (char *) ckd_calloc(strlen(inputfn) + strlen(outputfmt) + 5, sizeof(char));
      sprintf(local_outputfn, "%s.%s", inputfn, outputfmt);
      outputfnfree = TRUE;
    }

    /* Outputpath = outputdir . "/" (or "\" in windows). outputfn; */
    /* Length = strlen(outputdir) + 1 + strlen(outputfn) + 5 (For safety) */
    outputpath =
      (char *) ckd_calloc(strlen(outputdir) + strlen(outputfn) + 6, sizeof(char));


#if WIN32
    strcpy(separator, "\\");
#else
    strcpy(separator, "/");
#endif

    sprintf(outputpath, "%s%s%s", outputdir, separator, outputfn);
    lm_write(lm, outputpath, inputfn, outputfmt);


    if (local_outputfn) {
      ckd_free(local_outputfn);
    }
    ckd_free(outputpath);

    lm_free(lm);
    return 0;
}
