close all
clear();
clc();

%% LED DATA
% [led_spd, led_wls, led_names] = read_spd("input/LED_SPD_CALIBRATION.csv");

%% COLOR MATCHING FUNCTION
cmf10 = prepare_cmf("resources/ciexyz64_1.csv");

%% STANDARD VIS SPECIMEN
VIS = read_vis_specimen("resources/specimen_vis_standard.csv");
D50 = read_vis_specimen("resources/specimen_vis_D50.csv");
D55 = read_vis_specimen("resources/specimen_vis_D55.csv");
D65 = read_vis_specimen("resources/specimen_vis_D65.csv");
D75 = read_vis_specimen("resources/specimen_vis_D75.csv");

%% CIE STANDARD ILLUMINANTS
IL = read_illuminant("resources/standard_illuminants.csv");

%% CIE STANDARD ILLUMINANT COORDINATES
IL.D50.u = 0.2102;
IL.D50.v = 0.4889;
IL.D55.u = 0.2051;
IL.D55.v = 0.4816;
IL.D65.u = 0.1979;
IL.D65.v = 0.4695;
IL.D75.u = 0.1930;
IL.D75.v = 0.4601;

%% TCS SPECIMEN
TCS = read_tcs_specimen("resources/specimen_tcs.csv");

%% STANDARD UV SPECIMEN
UV = read_uv_fluorescence("resources/specimen_uv_standard.csv");
uvD50 = read_uv_specimen("resources/specimen_uv_D50.csv");
uvD55 = read_uv_specimen("resources/specimen_uv_D55.csv");
uvD65 = read_uv_specimen("resources/specimen_uv_D65.csv");
uvD75 = read_uv_specimen("resources/specimen_uv_D75.csv");

%% EXPORT
save("output/parameters.mat");