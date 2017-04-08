#include "core.h"
#include "string.h"
#include "stdlib.h"
#include "libvvplot_api.h"

void print_version()
{
    fprintf(stderr, "libvvplot.so compiled with:\n");
    fprintf(stderr, " - libvvhd git_commit %s\n", Space().getGitInfo());
    unsigned ver[3];
    H5get_libversion(&ver[0], &ver[1], &ver[2]);
    fprintf(stderr, " - libhdf version %u.%u.%u\n", ver[0], ver[1], ver[2]);
    fflush(stderr);
}

void print_help()
{
    fprintf(stderr, "Usage: libvvplot {-h,-v,-p,-L,-m,-M,-V,-I} FILE DATASET [ARGS]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, " -h : show this message\n");
    fprintf(stderr, " -v : show version info\n");

    fprintf(stderr, " -p : extract DATASET as plain text\n");
    fprintf(stderr, " -L : extract DATASET as binary matrix\n");
    fprintf(stderr, " -m : extract DATASET as binary matrix with prepended XMIN XMAX YMIN YMAX SPACING (double)\n");
    fprintf(stderr, " -M : extract DATASET as binary matrix in form (x, y, value)\n");

    fprintf(stderr, " -V : calculate velocity; DATASET ignored; ARGS: px1,py1 [px2,py2 ...]\n");
    fprintf(stderr, " -P : calculate pressure; DATASET ignored; ARGS: px1,py1 [px2,py2 ...]\n");
    fprintf(stderr, " -I : plot isolines on a DATASET; ARGS: c1 [c2 ...]\n");

    fprintf(stderr, " --vorticity : plot map_vorticity(); DATASET ignored; ARGS: XMIN XMAX YMIN YMAX SPACING\n");

    fflush(stderr);
}


int main(int argc, char **argv)
{
    /**/ if (argc<2) { print_help(); exit(1); }
    else if (!strcmp(argv[1], "-h")) { print_help(); exit(0); }
    else if (!strcmp(argv[1], "-v")) { print_version(); exit(0); }
    else if (argc < 4) { print_help(); exit(1); }
    else if (!strcmp(argv[1], "-p") ||
             !strcmp(argv[1], "-L") ||
             !strcmp(argv[1], "-m") ||
             !strcmp(argv[1], "-M") ||
             !strcmp(argv[1], "-V") ||
             !strcmp(argv[1], "-P") ||
             !strcmp(argv[1], "-I") ||
             !strcmp(argv[1], "--vorticity")
             ) {;}
    else {fprintf(stderr, "Bad option '%s'. See '-h' for help.\n", argv[1]); exit(-1); }

    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
    hid_t fid = H5Fopen(argv[2], H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fid < 0)
    {
        H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
        H5Eprint2(H5E_DEFAULT, stderr);
        fprintf(stderr, "error: argument FILE: can't open file '%s'\n", argv[2]);
        return 2;
    }

    if (!strcmp(argv[1], "-p"))
    {
        dset_print(fid, argv[3]);
    }
    else if (!strcmp(argv[1], "-L"))
    {
        list_extract(fid, argv[3]);
    }
    else if (!strcmp(argv[1], "-m"))
    {
        map_extract(fid, argv[3], binary_mode::matrix);
    }
    else if (!strcmp(argv[1], "-M"))
    {
        map_extract(fid, argv[3], binary_mode::xyvalue);
    }

    else if (!strcmp(argv[1], "-V"))
    {
        TVec* points = (TVec*)malloc((argc-4)*sizeof(TVec));
        for (int i=4; i<argc; i++)
        {
            int len;
            if (sscanf(argv[i], "%lf,%lf%n", &points[i-4].x, &points[i-4].y, &len)!=2 || argv[i][len])
            {
                fprintf(stderr, "error: argument ARGS: bad point '%s': must be 'px,py'\n", argv[i]);
                exit(-1);
            }
        }
        velocity_print(fid, points, argc-4);
        free(points);
    }
    else if (!strcmp(argv[1], "-P"))
    {
        TVec* points = (TVec*)malloc((argc-4)*sizeof(TVec));
        for (int i=4; i<argc; i++)
        {
            int len;
            if (sscanf(argv[i], "%lf,%lf%n", &points[i-4].x, &points[i-4].y, &len)!=2 || argv[i][len])
            {
                fprintf(stderr, "error: argument ARGS: bad point '%s': must be 'px,py'\n", argv[i]);
                exit(-1);
            }
        }

        pressure_print(fid, points, argc-4);
        free(points);
    }
    else if (!strcmp(argv[1], "-I"))
    {
        float* vals = (float*)malloc((argc-4)*sizeof(float));
        for (int i=4; i<argc; i++)
        {
            int len;
            if (sscanf(argv[i], "%f%n", &vals[i-4], &len)!=1 || argv[i][len])
            {
                fprintf(stderr, "error: argument ARGS: must be float");
                exit(-1);
            }
        }
        map_isoline(fid, argv[3], vals, argc-4);
        free(vals);
    }

    else if (!strcmp(argv[1], "--vorticity"))
    {
        if (argc != 9)
        {
            fprintf(stderr, "error: argument ARGS: must be XMIN XMAX YMIN YMAX SPACING\n");
            exit(-1);
        }

        H5Fclose(fid);
        fid = H5Fopen(argv[2], H5F_ACC_RDWR, H5P_DEFAULT);
        if (fid < 0)
        {
            H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
            H5Eprint2(H5E_DEFAULT, stderr);
            fprintf(stderr, "error: argument FILE: can't reopen file '%s' for writing\n", argv[2]);
            return 2;
        }

        double args[5];
        for (int i=4; i<9; i++)
        {
            int len;
            if (sscanf(argv[i], "%lf%n", &args[i-4], &len)!=1 || argv[i][len])
            {
                fprintf(stderr, "error: argument ARGS: must be float");
                exit(-1);
            }
        }
        map_vorticity(fid, args[0], args[1], args[2], args[3], args[4]);
    }

    H5Fclose(fid);

    exit(0);
}
