% build_dll.m
% Generates the C++ DLL from optimize_led_weights.m and evaluate_metrics.m using MATLAB Coder

% 1. Create configuration object for DLL
cfg = coder.config('dll');
cfg.TargetLang = 'C++';
cfg.FilePartitionMethod = 'SingleFile'; % Optional: keeps output clean

% 2. Define input types for both functions
num_wls = 81;   % 81 wavelength steps (380 to 780 by 5nm = 81 points)
num_leds = 22;  % 22 LEDs

type_led_spd = coder.typeof(0, [num_wls, num_leds], [false, false]);
type_led_wls = coder.typeof(0, [num_wls, 1], [false, false]);
type_illuminant = coder.typeof(0);
type_weights = coder.typeof(0, [1, num_leds], [false, false]);

% New approach
%max_leds = 22;

%type_led_spd = coder.typeof( ...
%    0, ...
%    [num_wls, max_leds], ...
%    [false, true]);   % second dimension is variable (The second true means the number of LED columns can vary from 1 to 22.)

%type_weights = coder.typeof( ...
%    0, ...
%    [1, max_leds], ...
%    [false, true]);

% You may also need to enable dynamic memory allocation:
%cfg.DynamicMemoryAllocation = 'AllVariableSizeArrays';


args_opt = {type_led_spd, type_led_wls, type_illuminant};
args_eval = {type_weights, type_led_spd, type_led_wls, type_illuminant};

% 3. Generate Code
codegen optimize_led_weights.m -args args_opt evaluate_metrics.m -args args_eval -config cfg -report
