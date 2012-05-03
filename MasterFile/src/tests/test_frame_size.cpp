#include "vpxt_test_declarations.h"

int test_frame_size(int argc,
                    const char** argv,
                    const std::string &working_dir,
                    const std::string sub_folder_str,
                    int test_type,
                    int delete_ivf,
                    int artifact_detection)
{
    char *comp_out_str = "Frame Size";
    char *test_dir = "test_frame_size";
    int input_ver = vpxt_check_arg_input(argv[1], argc);

    if (input_ver < 0)
        return vpxt_test_help(argv[1], 0);

    std::string input = argv[2];
    int mode = atoi(argv[3]);
    int bitrate = atoi(argv[4]);
    int starting_width = atoi(argv[5]);
    int starting_height = atoi(argv[6]);
    std::string enc_format = argv[7];
    std::string dec_format = argv[8];

    int speed = 0;

    //////////// Formatting Test Specific directory ////////////
    std::string cur_test_dir_str; // <- All Options need to set a value for this
    std::string file_index_str;
    char main_test_dir_char[255] = "";
    char file_index_output_char[255] = "";

    if (initialize_test_directory(argc, argv, test_type, working_dir, test_dir,
        cur_test_dir_str, file_index_str, main_test_dir_char,
        file_index_output_char, sub_folder_str) == 11)
        return kTestErrFileMismatch;

    char input_file_name[256] = "";
    vpxt_file_name(input.c_str(), input_file_name, 1);

    std::string frame_size_base = cur_test_dir_str;
    frame_size_base += slashCharStr() + input_file_name;

    char new_width[20];
    char new_height[20];
    std::string raw_crop[47];
    std::string raw_ext;
    vpxt_get_file_extension(input.c_str(), raw_ext);

    // height
    int counter = 0;
    int file_num = 1;
    while (counter < 16)
    {
        vpxt_itoa_custom(starting_width, new_width, 10); // width
        vpxt_itoa_custom(starting_height - (counter), new_height, 10); // height

        raw_crop[file_num] = frame_size_base + "_";
        raw_crop[file_num] += new_width;
        raw_crop[file_num] +=  "x";
        raw_crop[file_num] += new_height;
        raw_crop[file_num] +=  "_raw" + raw_ext;

        ++counter;
        ++file_num;
    }

    // width
    counter = 1;
    while (counter < 16)
    {
        vpxt_itoa_custom(starting_width - (counter), new_width, 10); // width
        vpxt_itoa_custom(starting_height, new_height, 10); // height

        raw_crop[file_num] = frame_size_base + "_";
        raw_crop[file_num] += new_width;
        raw_crop[file_num] +=  "x";
        raw_crop[file_num] += new_height;
        raw_crop[file_num] += "_raw" + raw_ext;

        ++counter;
        ++file_num;
    }

    // width and height
    counter = 1;
    while (counter < 16)
    {
        vpxt_itoa_custom(starting_width - (counter), new_width, 10); // width
        vpxt_itoa_custom(starting_height - (counter), new_height, 10); // height

        raw_crop[file_num] = frame_size_base + "_";
        raw_crop[file_num] += new_width;
        raw_crop[file_num] += "x";
        raw_crop[file_num] += new_height;
        raw_crop[file_num] += "_raw" + raw_ext;

        ++counter;
        ++file_num;
    }

    // initilize artifact detection
    int enc_crop_art_det[47];
    file_num = 1;
    while(file_num < 47)
    {
        enc_crop_art_det[file_num] = artifact_detection;
        ++file_num;
    }

    // encoded file names
    file_num = 1;
    std::string enc_crop[47];
    while (file_num < 47)
    {
        enc_crop[file_num] = raw_crop[file_num];
        enc_crop[file_num].erase(enc_crop[file_num].end() - 7,
            enc_crop[file_num].end());
        enc_crop[file_num] += "enc";
        vpxt_enc_format_append(enc_crop[file_num], enc_format);
        ++file_num;
    }

    std::string text_file_str = cur_test_dir_str + slashCharStr() + test_dir;

    if (test_type == kCompOnly || test_type == kFullTest)
        text_file_str += ".txt";
    else
        text_file_str += "_TestOnly.txt";


    FILE *fp;

    if ((fp = freopen(text_file_str.c_str(), "w", stderr)) == NULL)
    {
        tprintf(PRINT_STD, "Cannot open out put file: %s\n",
            text_file_str.c_str());
        exit(1);
    }

    ////////////////////////////////
    //////////////////////////////////////////////////////////

    if (test_type == kFullTest)
        print_header_full_test(argc, argv, main_test_dir_char);

    if (test_type == kCompOnly)
        print_header_compression_only(argc, argv, main_test_dir_char);

    if (test_type == kTestOnly)
        print_header_test_only(argc, argv, cur_test_dir_str);

    // Make sure starting width and height are mults of 16
    if ((starting_width % 16 != 0) && (starting_height % 16 != 0))
    {
        tprintf(PRINT_BTH, "\nError: Starting width and height are not "
            "multiples of 16\n\nFailed\n");

        fclose(fp);
        record_test_complete(file_index_str, file_index_output_char, test_type);
        return kTestFailed;
    }

    if (starting_height % 16 != 0)
    {
        tprintf(PRINT_BTH, "\nError: Starting height is not a multiple of "
            "16\n\nFailed\n");

        fclose(fp);
        record_test_complete(file_index_str, file_index_output_char, test_type);
        return kTestFailed;
    }

    if (starting_width % 16 != 0)
    {
        tprintf(PRINT_BTH, "\nError: Starting width is not a multiple of "
            "16\n\nFailed\n");

        fclose(fp);
        record_test_complete(file_index_str, file_index_output_char, test_type);
        return kTestFailed;
    }

    vpxt_cap_string_print(PRINT_BTH, "%s", test_dir);

    VP8_CONFIG opt;
    vpxt_default_parameters(opt);

    /////////////////// Use Custom Settings ///////////////////
    if (input_ver == 2)
    {
        if (!vpxt_file_exists_check(argv[argc-1]))
        {
            tprintf(PRINT_BTH, "\nInput Settings file %s does not exist\n",
                argv[argc-1]);

            fclose(fp);
            record_test_complete(file_index_str, file_index_output_char,
                test_type);
            return kTestIndeterminate;
        }

        opt = vpxt_input_settings(argv[argc-1]);
        bitrate = opt.target_bandwidth;
    }

    /////////////////////////////////////////////////////////

    opt.target_bandwidth = bitrate;
    double psnr_arr[46];
    int raw_crop_num = 1;

    // Run Test only (Runs Test, Sets up test to be run, or skips compresion of
    // files)
    if (test_type == kTestOnly)
    {
        // Get Prexisting psnr values
        while (raw_crop_num < 47)
        {
            psnr_arr[raw_crop_num-1] = vpxt_get_psnr(
                enc_crop[raw_crop_num].c_str());
            ++raw_crop_num;
        }
    }
    else
    {
        // Create Raw Crops
        int x = 0;
        opt.Mode = mode;

        while (x < 16)
        {
            // Crop
            tprintf(PRINT_BTH, "\nCroping to %i %i", starting_width,
                starting_height - x);
            vpxt_crop_raw_clip(input.c_str(), raw_crop[raw_crop_num].c_str(), 0,
                0, starting_width, starting_height - x, 1, 1);

            // Comp
            char file_name_char[256];
            char file_name_char_2[256];
            snprintf(file_name_char, 256, raw_crop[raw_crop_num].c_str());
            vpxt_file_name(file_name_char, file_name_char_2, 1);

            tprintf(PRINT_BTH, "\n\nCompressing %s", file_name_char_2);

            if (vpxt_compress(raw_crop[raw_crop_num].c_str(),
                enc_crop[raw_crop_num].c_str(), speed, bitrate, opt,
                comp_out_str, 0, 0, enc_format) == -1)
            {
                fclose(fp);
                record_test_complete(file_index_str, file_index_output_char,
                    test_type);
                return kTestIndeterminate;
            }

            // PSNR
            psnr_arr[raw_crop_num-1] = vpxt_psnr(raw_crop[raw_crop_num].c_str(),
                enc_crop[raw_crop_num].c_str(), 0, PRINT_BTH, 1, 0, 0, 0, NULL,
                enc_crop_art_det[raw_crop_num]);

            std::string psnr_out_file;
            vpxt_remove_file_extension(enc_crop[raw_crop_num].c_str(),
                psnr_out_file);
            psnr_out_file += "psnr.txt";

            std::ofstream out_file_psnr(psnr_out_file.c_str());
            out_file_psnr << psnr_arr[raw_crop_num-1];
            out_file_psnr.close();

            // Delete(file deletions are done here due to the number of files
            // that need to be generated)
            if (delete_ivf)
            {
                vpxt_delete_files(1, raw_crop[raw_crop_num].c_str());
                vpxt_delete_files(1, enc_crop[raw_crop_num].c_str());
            }

            ++x;
            ++raw_crop_num;
        }

        x = 1;

        while (x < 16)
        {
            // Crop
            tprintf(PRINT_BTH, "\nCroping to %i %i", starting_width - x,
                starting_height);
            vpxt_crop_raw_clip(input.c_str(), raw_crop[raw_crop_num].c_str(), 0,
                0, starting_width - x, starting_height, 1, 1);

            // Comp
            char file_name_char[256];
            char file_name_char_2[256];
            snprintf(file_name_char, 256, enc_crop[raw_crop_num].c_str());
            vpxt_file_name(file_name_char, file_name_char_2, 1);

            tprintf(PRINT_BTH, "\nCompressing %s", file_name_char_2);

            if (vpxt_compress(raw_crop[raw_crop_num].c_str(),
                enc_crop[raw_crop_num].c_str(), speed, bitrate, opt,
                comp_out_str, 0, 0, enc_format) == -1)
            {
                fclose(fp);
                record_test_complete(file_index_str, file_index_output_char,
                    test_type);
                return kTestIndeterminate;
            }

            // PSNR
            psnr_arr[raw_crop_num-1] = vpxt_psnr(raw_crop[raw_crop_num].c_str(),
                enc_crop[raw_crop_num].c_str(), 0, PRINT_BTH, 1, 0, 0, 0, NULL,
                enc_crop_art_det[raw_crop_num]);

            std::string psnr_out_file;
            vpxt_remove_file_extension(enc_crop[raw_crop_num].c_str(),
                psnr_out_file);
            psnr_out_file += "psnr.txt";

            std::ofstream out_file_psnr(psnr_out_file.c_str());
            out_file_psnr << psnr_arr[raw_crop_num-1];
            out_file_psnr.close();

            // Delete
            if (delete_ivf)
            {
                vpxt_delete_files(1, raw_crop[raw_crop_num].c_str());
                vpxt_delete_files(1, enc_crop[raw_crop_num].c_str());
            }

            ++x;
            ++raw_crop_num;
        }

        x = 1;

        while (x < 16)
        {
            // Crop
            tprintf(PRINT_BTH, "\nCroping to %i %i", starting_width - x,
                starting_height - x);
            vpxt_crop_raw_clip(input.c_str(), raw_crop[raw_crop_num].c_str(), 0,
                0, starting_width - x, starting_height - x, 1, 1);

            // Comp
            char file_name_char[256];
            char file_name_char_2[256];
            snprintf(file_name_char, 256, raw_crop[raw_crop_num].c_str());
            vpxt_file_name(file_name_char, file_name_char_2, 1);

            tprintf(PRINT_BTH, "\nCompressing %s", file_name_char_2);

            if (vpxt_compress(raw_crop[raw_crop_num].c_str(),
                enc_crop[raw_crop_num].c_str(), speed, bitrate, opt,
                comp_out_str, 0, 0, enc_format) == -1)
            {
                fclose(fp);
                record_test_complete(file_index_str, file_index_output_char,
                    test_type);
                return kTestIndeterminate;
            }

            // PSNR
            psnr_arr[raw_crop_num-1] = vpxt_psnr(raw_crop[raw_crop_num].c_str(),
                enc_crop[raw_crop_num].c_str(), 0, PRINT_BTH, 1, 0, 0, 0, NULL,
                enc_crop_art_det[raw_crop_num]);

            std::string psnr_out_file;
            vpxt_remove_file_extension(enc_crop[raw_crop_num].c_str(),
                psnr_out_file);
            psnr_out_file += "psnr.txt";

            std::ofstream out_file_psnr(psnr_out_file.c_str());
            out_file_psnr << psnr_arr[raw_crop_num-1];
            out_file_psnr.close();

            // Delete
            if (delete_ivf)
            {
                vpxt_delete_files(1, raw_crop[raw_crop_num].c_str());
                vpxt_delete_files(1, enc_crop[raw_crop_num].c_str());
            }

            ++x;
            ++raw_crop_num;
        }
    }

    if (test_type == kCompOnly)
    {
        fclose(fp);
        record_test_complete(file_index_str, file_index_output_char, test_type);
        return kTestEncCreated;
    }

    int percent_fail = 0;
    int min_psnr_fail = 0;

    raw_crop_num = 1;
    double five_percent_psnr = (5 * psnr_arr[0]) / 100;

    while (raw_crop_num < 47)
    {
        char file_name_char[256];
        char file_name_char_2[256];
        snprintf(file_name_char, 256, enc_crop[raw_crop_num].c_str());
        vpxt_file_name(file_name_char, file_name_char_2, 0);

        if (raw_crop_num == 1)
        {
            tprintf(PRINT_STD, "\n\n PSNR %s: %.2f", file_name_char_2,
                psnr_arr[raw_crop_num-1]);
        }
        else if (psnr_arr[raw_crop_num-1] <  psnr_arr[0] + five_percent_psnr &&
            psnr_arr[raw_crop_num-1] >  psnr_arr[0] - five_percent_psnr)
        {
            tprintf(PRINT_BTH, "\n PSNR %s: %.2f within 5%% of %.2f - Passed",
                file_name_char_2, psnr_arr[raw_crop_num-1], psnr_arr[0]);
        }
        else
        {
            tprintf(PRINT_BTH, "\n PSNR %s: %.2f not within 5%% of %.2f - "
                "Failed", file_name_char_2, psnr_arr[raw_crop_num-1],
                psnr_arr[0]);
            percent_fail = 1;
        }

        ++raw_crop_num;
    }

    tprintf(PRINT_BTH, "\n\n");

    raw_crop_num = 1;

    while (raw_crop_num < 47)
    {
        char file_name_char[256];
        char file_name_char_2[256];
        snprintf(file_name_char, 256, enc_crop[raw_crop_num].c_str());
        vpxt_file_name(file_name_char, file_name_char_2, 0);

        if (psnr_arr[raw_crop_num-1] > 25.0)
        {
            tprintf(PRINT_BTH, "\n PSNR %s: %.2f > %.2f - Passed",
                file_name_char_2, psnr_arr[raw_crop_num-1], 25.0);
        }
        else
        {
            tprintf(PRINT_BTH, "\n PSNR %s: %.2f < %.2f - Failed",
                file_name_char_2, psnr_arr[raw_crop_num-1], 25.0);
            min_psnr_fail = 1;
        }

        ++raw_crop_num;
    }

    int test_state = kTestPassed;
    tprintf(PRINT_BTH, "\n\n\nResults:\n\n");

    if (percent_fail == 0)
    {
        vpxt_formated_print(RESPRT, "All PSNRs are within 5%% of %.2f - "
            "Passed", psnr_arr[0]);
        tprintf(PRINT_BTH, "\n");
    }
    else
    {
        vpxt_formated_print(RESPRT, "Not all PSNRs are within 5%% of %.2f - "
            "Failed", psnr_arr[0]);
        tprintf(PRINT_BTH, "\n");
        test_state = kTestFailed;
    }

    if (min_psnr_fail == 0)
    {
        vpxt_formated_print(RESPRT, "All PSNRs are greater than 25.0 - Passed");
        tprintf(PRINT_BTH, "\n");
    }
    else
    {
        vpxt_formated_print(RESPRT, "Not all PSNRs are greater than 25.0 - "
            "Failed");
        tprintf(PRINT_BTH, "\n");
        test_state = kTestFailed;
    }

    // handle possible artifact
    file_num = 1;
    while(file_num < 47)
    {
        if(enc_crop_art_det[file_num] == kPossibleArtifactFound)
        {
            tprintf(PRINT_BTH, "\nPossible Artifact\n");

            fclose(fp);
            record_test_complete(file_index_str, file_index_output_char,
                test_type);
            return kTestPossibleArtifact;
        }

        ++file_num;
    }

    if (test_state == kTestPassed)
        tprintf(PRINT_BTH, "\nPassed\n");
    if (test_state == kTestFailed)
        tprintf(PRINT_BTH, "\nFailed\n");

    fclose(fp);
    record_test_complete(file_index_str, file_index_output_char, test_type);
    return test_state;
}