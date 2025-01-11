#include <stdio.h>
#include <math.h>

#define CYCLE           2
#define DATA_LENGTH     20
#define PI              3.141592f
#define NOM_FREQ_GRID   50.0f       // Nominal Grid Frequency
#define NOM_PEAK_VOLT   311.0f      // Nominal Peak Voltage  

float Va[] = {
    156.63, 246.59, 294.72, 305.51, 300.66,
    268.03, 204.18, 125.41, 42.954, -48.322,
    -154.08, -243.95, -293.12, -303.09, -297.98,
    -264.13, -202.1, -122.25, -39.893, 51.818,
	156.63, 246.59, 294.72, 305.51, 300.66,
    268.03, 204.18, 125.41, 42.954, -48.322,
    -154.08, -243.95, -293.12, -303.09, -297.98,
    -264.13, -202.1, -122.25, -39.893, 51.818
};

float Vb[] = {
    -308.4, -280.19, -240.66, -186.6, -99.744,
    -0.54547, 92.853, 181.46, 262.05, 312.39,
    311.44, 283.76, 245.04, 188.62, 102.16,
    2.9662, -89.395, -176.17, -259.16, -309.96,
	-308.4, -280.19, -240.66, -186.6, -99.744,
    -0.54547, 92.853, 181.46, 262.05, 312.39,
    311.44, 283.76, 245.04, 188.62, 102.16,
    2.9662, -89.395, -176.17, -259.16, -309.96
};

float Vc[] = {
    156.11, 82.694, -21.783, -128.37, -213.06,
    -269.49, -309.58, -313.4, -273.73, -214.81,
    -154.29, -79.64, 24.679, 132.16, 216.63,
    274.14, 311.11, 315.76, 276.27, 216.22,
	156.11, 82.694, -21.783, -128.37, -213.06,
    -269.49, -309.58, -313.4, -273.73, -214.81,
    -154.29, -79.64, 24.679, 132.16, 216.63,
    274.14, 311.11, 315.76, 276.27, 216.22
};

float harmonic[] = {0.0};

typedef struct _DDATA {
    float *in_a;
    float *in_b;
    float *in_c;
    float F_est;
    float Theta_est;
    float *Harmonics;
    float Ts;
    float Kc1; 
    float Kc2; 
    float Kc3;
} DDATA;

DDATA ddata = {
	.in_a = Va,
    .in_b = Vb,
    .in_c = Vc,
    .Harmonics = harmonic,
    .Theta_est = 0.0f,
    .Ts = 0.001f,
    .Kc1 = 222.1603f,       // Kp gains for the PLL's PI controller
    .Kc2 = 25181.0f,        // Ki gains for the PLL's PI controller
};

//
// Estimate frequency and theta
//
void estimateFrequencyAndTheta(DDATA *ddata, int dataSize) {
    float w0 = 0.0f, w1 = 0.0f, out_PLL = 0.0f, uVQ = 0.0f, temp_error = 0.0f;
    
    for (int n = 0; n < dataSize; n++) {

        // Per-unit normalization of the voltage inputs
        float a = ddata->in_a[n] / NOM_PEAK_VOLT;
        float b = ddata->in_b[n] / NOM_PEAK_VOLT;
        float c = ddata->in_c[n] / NOM_PEAK_VOLT;

        // SRF-PLL (Synchronous Reference Frame Phase-Locked Loop)
        // Using the rotating frame to find the Q component
        uVQ = (-2.0f / 3.0f) * (sinf(ddata->Theta_est) * a + 
               sinf(ddata->Theta_est - 2.0f * PI / 3.0f) * b +
               sinf(ddata->Theta_est + 2.0f * PI / 3.0f) * c);

        // Update PLL output
        out_PLL += ddata->Kc1 * (uVQ - temp_error) + 
                    ddata->Kc2 * ddata->Ts * (uVQ + temp_error) * 0.5f;
        temp_error = uVQ;

        // Update the angular frequency (omega)
        w1 = w0;
        w0 = out_PLL + 2.0f * PI * NOM_FREQ_GRID;

        // Estimate theta from the omega
        ddata->Theta_est += ((w0 + w1) * ddata->Ts / 2.0f);
        
        // Ensure theta stays within the range [0, 2π]
        if (ddata->Theta_est >= 2.0f * PI) {
            ddata->Theta_est -= 2.0f * PI;
        }

        // Calculate the Frequency from the omega
        ddata->F_est = w0 / (2.0f * PI);  
    }
}

//
// Get Harmonic Amplitudes
//
void getHarmonicAmplitudes(DDATA *ddata, int dataSize) {
    // Number of samples per cycle
    dataSize = dataSize/CYCLE;          //20 samples = 1000 Hz (sampling rate)/ 50 Hz (nominal grid frequency)
    for (int k = 1; k <= 5; k++) { 
        float real = 0, imag = 0;

        for (int n = 0; n < dataSize; n++) {
            real += ddata->in_a[n] * cosf(2 * PI * k * (n) / dataSize); // Sum of real part of the k-th harmonic
            imag -= ddata->in_a[n] * sinf(2 * PI * k * (n) / dataSize); // Sum of imaginary part of the k-th harmonic   
        }
        
        // Calculate the amplitude of the k-th harmonic
        ddata->Harmonics[k-1] = sqrt(real * real + imag * imag) / (dataSize / 2);
    }
}

int main() {

}

