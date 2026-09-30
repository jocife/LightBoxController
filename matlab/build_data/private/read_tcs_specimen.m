function tcs = read_tcs_specimen(filename)
    % Read TCS data from CSV
    % Wavelength;TCS01;TCS02;TCS03;TCS04;TCS05;TCS06;TCS07;TCS08
    opts = detectImportOptions(filename, 'NumHeaderLines', 0, 'DecimalSeparator', '.');
    t = readtable(filename, opts);
    
    tcs.wl = t{:, 1};
    tcs.TCS01 = t{:, 2};
    tcs.TCS02 = t{:, 3};
    tcs.TCS03 = t{:, 4};
    tcs.TCS04 = t{:, 5};
    tcs.TCS05 = t{:, 6};
    tcs.TCS06 = t{:, 7};
    tcs.TCS07 = t{:, 8};
    tcs.TCS08 = t{:, 9};
end
