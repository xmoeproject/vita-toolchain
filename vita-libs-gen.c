#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vita-import.h"

void usage();
int generate_assembly(vita_imports_t *imports);

int main(int argc, char *argv[])
{
	if (argc < 2) {
		usage();
		goto exit_failure;
	}

	vita_imports_t *imports = vita_imports_load(argv[1]);

	if (imports == NULL) {
		goto exit_failure;
	}

	if (!generate_assembly(imports)) {
		fprintf(stderr, "Error generating the assembly file\n");
	}

	vita_imports_free(imports);

	return EXIT_SUCCESS;
exit_failure:
	return EXIT_FAILURE;
}


int generate_assembly(vita_imports_t *imports)
{
	char filename[128];
	FILE *fp;
	int i, j, k;

	for (i = 0; i < imports->n_libs; i++) {

		for (j = 0; j < imports->libs[i]->n_modules; j++) {

			snprintf(filename, 128, "%s.S", imports->libs[i]->modules[j]->name);

			if ((fp = fopen(filename, "w")) == NULL) {
				return 0;
			}

			fprintf(fp, "@ %s\n", filename);
			fprintf(fp, "@ this file was automagically generated by vita-libs-gen by xerpi\n\n");
			fprintf(fp, "@ module: %s\n\n", imports->libs[i]->modules[j]->name);
			fprintf(fp, ".arch armv7−a\n\n");

			if (imports->libs[i]->modules[j]->n_functions > 0)
				fprintf(fp, ".section .vitalink.fstubs,\"ax\",%%progbits\n\n");

			for (k = 0; k < imports->libs[i]->modules[j]->n_functions; k++) {

				const char *fname = imports->libs[i]->modules[j]->functions[k]->name;
				fprintf(fp,
					"\t.align 4\n"
					"\t.global %s\n"
					"\t.type %s, %%function\n"
					"%s:\n"
					"\t.word 0x%08X\n"
					"\t.word 0x%08X\n"
					"\t.word 0x%08X\n\n",
					fname, fname, fname,
					imports->libs[i]->NID,
					imports->libs[i]->modules[j]->NID,
					imports->libs[i]->modules[j]->functions[k]->NID);
			}
			fprintf(fp, "\n");

			if (imports->libs[i]->modules[j]->n_variables > 0)
				fprintf(fp, ".section .vitalink.vstubs,\"awx\",%%progbits\n\n");

			for (k = 0; k < imports->libs[i]->modules[j]->n_variables; k++) {

				const char *vname = imports->libs[i]->modules[j]->variables[k]->name;
				fprintf(fp,
					"\t.align 4\n"
					"\t.global %s\n"
					"\t.type %s, %%object\n"
					"%s:\n"
					"\t.word 0x%08X\n"
					"\t.word 0x%08X\n"
					"\t.word 0x%08X\n\n",
					vname, vname, vname,
					imports->libs[i]->NID,
					imports->libs[i]->modules[j]->NID,
					imports->libs[i]->modules[j]->variables[k]->NID);
			}

			fprintf(fp, "@ end of the file\n");
			fclose(fp);
		}
	}

	return 1;
}

void usage()
{
	fprintf(stderr,
		"vita-libs-gen by xerpi\n"
		"usage:\n\tvita-libs-gen in.json\n"
	);
}