/*
 * read a zone file from disk and prints it, one RR per line
 *
 * (c) NLnetLabs 2005-2008
 *
 * See the file LICENSE for the license
 */

#include "config.h"
#include <unistd.h>
#include <stdlib.h>

#include <ldns/ldns.h>

#include <errno.h>

int
main(int argc, char **argv)
{
	char *filename;
	FILE *fp;
	ldns_zone *z;
	int line_nr = 0;
	int c;
	bool canonicalize = false;
	bool sort = false;
	bool strip = false;
	bool only_dnssec = false;
	bool print_soa = true;
	ldns_status s;
	size_t i;
	ldns_rr_list *stripped_list;
	ldns_rr *cur_rr;
	ldns_rr_type cur_rr_type;

        while ((c = getopt(argc, argv, "cdhnsvz")) != -1) {
                switch(c) {
                	case 'c':
                		canonicalize = true;
                		break;
                	case 'd':
                		only_dnssec = true;
                		if (strip) {
                			fprintf(stderr, "Warning: stripping both DNSSEC and non-DNSSEC records. Output will be sparse.\n");
				}
				break;
			case 'h':
				printf("Usage: %s [-c] [-v] [-z] <zonefile>\n", argv[0]);
				printf("\tReads the zonefile and prints it.\n");
				printf("\tThe RR count of the zone is printed to stderr.\n");
				printf("\t-c canonicalize all rrs in the zone.\n");
				printf("\t-d only show DNSSEC data from the zone\n");
				printf("\t-h show this text\n");
				printf("\t-n do not print the SOA record\n");
				printf("\t-s strip DNSSEC data from the zone\n");
				printf("\t-v shows the version and exits\n");
				printf("\t-z sort the zone (implies -c).\n");
				printf("\nif no file is given standard input is read\n");
				exit(EXIT_SUCCESS);
				break;
			case 'n':
				print_soa = false;
				break;
                        case 's':
                        	strip = true;
                		if (only_dnssec) {
                			fprintf(stderr, "Warning: stripping both DNSSEC and non-DNSSEC records. Output will be sparse.\n");
				}
                        	break;
			case 'v':
				printf("read zone version %s (ldns version %s)\n", LDNS_VERSION, ldns_version());
				exit(EXIT_SUCCESS);
				break;
                        case 'z':
                		canonicalize = true;
                                sort = true;
                                break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		fp = stdin;
	} else {
		filename = argv[0];

		fp = fopen(filename, "r");
		if (!fp) {
			fprintf(stderr, "Unable to open %s: %s\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	
	s = ldns_zone_new_frm_fp_l(&z, fp, NULL, 0, LDNS_RR_CLASS_IN, &line_nr);

	if (strip) {
		stripped_list = ldns_rr_list_new();
		while ((cur_rr = ldns_rr_list_pop_rr(ldns_zone_rrs(z)))) {
			cur_rr_type = ldns_rr_get_type(cur_rr);
			if (cur_rr_type == LDNS_RR_TYPE_RRSIG ||
			    cur_rr_type == LDNS_RR_TYPE_NSEC ||
			    cur_rr_type == LDNS_RR_TYPE_NSEC3 ||
			    cur_rr_type == LDNS_RR_TYPE_NSEC3PARAMS
			   ) {
				ldns_rr_free(cur_rr);
			} else {
				ldns_rr_list_push_rr(stripped_list, cur_rr);
			}
		}
		ldns_rr_list_free(ldns_zone_rrs(z));
		ldns_zone_set_rrs(z, stripped_list);
	}
	if (only_dnssec) {
		stripped_list = ldns_rr_list_new();
		while ((cur_rr = ldns_rr_list_pop_rr(ldns_zone_rrs(z)))) {
			cur_rr_type = ldns_rr_get_type(cur_rr);
			if (cur_rr_type == LDNS_RR_TYPE_RRSIG ||
			    cur_rr_type == LDNS_RR_TYPE_NSEC ||
			    cur_rr_type == LDNS_RR_TYPE_NSEC3 ||
			    cur_rr_type == LDNS_RR_TYPE_NSEC3PARAMS
			   ) {
				ldns_rr_list_push_rr(stripped_list, cur_rr);
			} else {
				ldns_rr_free(cur_rr);
			}
		}
		ldns_rr_list_free(ldns_zone_rrs(z));
		ldns_zone_set_rrs(z, stripped_list);
	}

	if (s == LDNS_STATUS_OK) {
		if (canonicalize) {
			ldns_rr2canonical(ldns_zone_soa(z));
			for (i = 0; i < ldns_rr_list_rr_count(ldns_zone_rrs(z)); i++) {
				ldns_rr2canonical(ldns_rr_list_rr(ldns_zone_rrs(z), i));
			}
		}
		if (sort) {
			ldns_zone_sort(z);
		}

		if (print_soa && ldns_zone_soa(z)) {
			ldns_rr_print(stdout, ldns_zone_soa(z));
		}
		ldns_rr_list_print(stdout, ldns_zone_rrs(z));

		ldns_zone_deep_free(z);
	} else {
		fprintf(stderr, "%s at %d\n", 
				ldns_get_errorstr_by_id(s),
				line_nr);
                exit(EXIT_FAILURE);
	}
	fclose(fp);

        exit(EXIT_SUCCESS);
}
