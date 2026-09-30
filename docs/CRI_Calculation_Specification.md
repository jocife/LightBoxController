# CRI (Color Rendering Index) Calculation Specification

## 1. Input Data Requirements (CSV Format)
* **`led_spd.csv`**: Spectral Power Distribution of the test light source (LEDs of the lightbox). Columns: `wavelength`, `value`.
* **`cmf.csv`**: CIE 1931 2-degree Color Matching Functions. Columns: `wavelength`, `x_bar`, `y_bar`, `z_bar`.
* **`tcs.csv`**: Spectral reflectance of Test Color Samples (TCS1 to TCS8 for $R_a$, up to TCS14 for extended $R_i$). Columns: `wavelength`, `tcs1`, ..., `tcs14`.

*Note: Standard wavelength range is 380 nm to 780 nm at 5 nm intervals.*

## 2. Calculation Pipeline

### Step 1: Base Tristimulus Values (XYZ)
Calculate the XYZ values for the test source $S_t(\lambda)$.

$$X = k \sum S_t(\lambda) \bar{x}(\lambda) \Delta\lambda$$
$$Y = k \sum S_t(\lambda) \bar{y}(\lambda) \Delta\lambda$$
$$Z = k \sum S_t(\lambda) \bar{z}(\lambda) \Delta\lambda$$
$$k = \frac{100}{\sum S_t(\lambda) \bar{y}(\lambda) \Delta\lambda}$$

### Step 2: Chromaticity Coordinates (CIE 1960)
Convert XYZ to CIE 1960 uniform color space (u, v).

$$u = \frac{4X}{X + 15Y + 3Z}$$
$$v = \frac{6Y}{X + 15Y + 3Z}$$

### Step 3: Reference Illuminant Determination
1. Determine the Correlated Color Temperature (CCT) of the test source.
2. Select the reference illuminant $S_r(\lambda)$:
   * If CCT < 5000K: Use Planckian radiator equation.
   * If CCT $\ge$ 5000K: Use CIE Daylight (D-series) equations.

### Step 4: TCS Tristimulus Values
Calculate XYZ values for each TCS $i$ under both the test source ($X_{t,i}, Y_{t,i}, Z_{t,i}$) and the reference illuminant ($X_{r,i}, Y_{r,i}, Z_{r,i}$).

$$X_{t,i} = k_t \sum S_t(\lambda) \rho_i(\lambda) \bar{x}(\lambda) \Delta\lambda$$
*(Repeat for Y, Z and for the reference illuminant $S_r$)*

### Step 5: Chromatic Adaptation (von Kries)
Adapt the TCS coordinates under the test source to the reference illuminant reference frame to account for human visual adaptation. Convert adapted coordinates to $u_i', v_i'$.

### Step 6: CIE 1964 Uniform Color Space Transformation
Calculate $W^*, U^*, V^*$ for both test (adapted) and reference conditions.

$$W_i^* = 25 Y_i^{1/3} - 17$$
$$U_i^* = 13 W_i^* (u_i - u_{ref})$$
$$V_i^* = 13 W_i^* (v_i - v_{ref})$$

### Step 7: Color Difference Calculation ($\Delta E_i$)
Compute the Euclidean distance in the $W^* U^* V^*$ space for each sample.

$$\Delta E_i = \sqrt{(U_{t,i}^* - U_{r,i}^*)^2 + (V_{t,i}^* - V_{r,i}^*)^2 + (W_{t,i}^* - W_{r,i}^*)^2}$$

### Step 8: Special and General CRI ($R_i$ and $R_a$)
Calculate the special CRI for each sample:
$$R_i = 100 - 4.6 \Delta E_i$$

Calculate the General CRI ($R_a$) as the arithmetic mean of the first 8 samples:
$$R_a = \frac{1}{8} \sum_{i=1}^{8} R_i$$
