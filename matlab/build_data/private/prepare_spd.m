function spd = prepare_spd(spectrum, wavelengths)

    % spd.wl = (300:5:780).';
    % spd.val = interp1(wavelengths, spectrum, spd.wl, "linear", "extrap");
    % spd.val = max(spd.val, 0);

    % 1. Create the wavelength vector as a separate variable
    target_wl = (300:5:780).';
    
    % 2. Calculate the values using the temporary variable
    %    (Now we are not reading from 'spd' before it is fully defined)
    raw_val = interp1(wavelengths, spectrum, target_wl, "linear", "extrap");
    
    % 3. Apply the max function
    final_val = max(raw_val, 0);
    
    % 4. Build the structure all at once (or field by field safely)
    spd.wl = target_wl;
    spd.val = final_val;
    
end