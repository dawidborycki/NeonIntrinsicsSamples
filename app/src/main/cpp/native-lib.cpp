#include <jni.h>
#include <string>
#include <stdio.h>
#include <numeric>
#include "math.h"
#include <algorithm>
#include <chrono>

using namespace std;

// You can set this macro to 0 to test the code in the emulator
// or on the x86 platform
#define HAVE_NEON 1

#if HAVE_NEON
    #include "arm_neon.h"
#endif

#define SIGNAL_LENGTH 1024
#define SIGNAL_AMPLITUDE 100
#define NOISE_AMPLITUDE 25
#define KERNEL_LENGTH 16
#define TRANSFER_SIZE 16
#define THRESHOLD 50

// Kernel
int8_t kernel[KERNEL_LENGTH] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// Global variables
int8_t inputSignal[SIGNAL_LENGTH];
int8_t inputSignalTruncate[SIGNAL_LENGTH];
int8_t inputSignalConvolution[SIGNAL_LENGTH];

double processingTime;

void generateSignal() {
    auto phaseStep = 2 * M_PI / SIGNAL_LENGTH;

    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        auto phase = i * phaseStep;
        auto noise = rand() % NOISE_AMPLITUDE;

        inputSignal[i] = static_cast<int8_t>(SIGNAL_AMPLITUDE * sin(phase) + noise);
    }
}

void truncate() {
    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        inputSignalTruncate[i] = std::min(inputSignal[i], (int8_t)THRESHOLD);
    }
}

int getSum(int8_t* input, int length) {
    int sum = 0;

    for(int i = 0; i < length; i++) {
        sum += input[i];
    }

    return sum;
}

void convolution() {
    auto offset = -KERNEL_LENGTH / 2;

    // Get kernel sum (for normalization)
    auto kernelSum = getSum(kernel, KERNEL_LENGTH);

    // Calculate convolution
    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        int convSum = 0;

        for (int j = 0; j < KERNEL_LENGTH; j++) {
            convSum += kernel[j] * inputSignal[i + offset + j];
        }

        inputSignalConvolution[i] = (uint8_t)(convSum / kernelSum);
    }
}

#if HAVE_NEON

void truncateNeon() {
    // Duplicate threshValue
    int8x16_t threshValueNeon = vdupq_n_s8(THRESHOLD);

    for (int i = 0; i < SIGNAL_LENGTH; i += TRANSFER_SIZE) {
        // Load signal to registers
        int8x16_t inputNeon = vld1q_s8(inputSignal + i);

        // Truncate
        uint8x16_t partialResult = vminq_s8(inputNeon, threshValueNeon);

        // Store result in the output buffer
        vst1q_s8(inputSignalTruncate + i, partialResult);
    }
}

void convolutionNeon() {
    auto offset = -KERNEL_LENGTH / 2;

    // Get kernel sum (for normalization)
    auto kernelSum = getSum(kernel, KERNEL_LENGTH);

    // Load kernel
    int8x16_t kernelNeon = vld1q_s8(kernel);

    // Buffer for multiplication result
    int8_t *mulResult = new int8_t[TRANSFER_SIZE];

    // Calculate convolution
    for (int i = 0; i < SIGNAL_LENGTH; i++) {

        // Load input
        int8x16_t inputNeon = vld1q_s8(inputSignal + i + offset);

        // Multiply
        int8x16_t mulResultNeon = vmulq_s8(inputNeon, kernelNeon);

        // Store and accumulate
        // On A64 the following lines of code can be replaced by a single instruction
        // auto convSum = vaddvq_s8(mulResultNeon)
        vst1q_s8(mulResult, mulResultNeon);
        auto convSum = getSum(mulResult, TRANSFER_SIZE);

        // Store result
        inputSignalConvolution[i] = (uint8_t) (convSum / kernelSum);
    }

    delete mulResult;
}

#endif

double usElapsedTime(chrono::system_clock::time_point start) {
    auto end = chrono::system_clock::now();

    return chrono::duration_cast<chrono::microseconds>(end - start).count();
}

chrono::system_clock::time_point now() {
    return chrono::system_clock::now();
}

jbyteArray nativeBufferToByteArray(JNIEnv *env, int8_t* buffer, int length) {
    auto byteArray = env->NewByteArray(length);

    env->SetByteArrayRegion(byteArray, 0, length, buffer);

    return byteArray;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_neonintrinsicssamples_MainActivity_generateSignal(JNIEnv *env, jobject /* this */) {
    generateSignal();

    return nativeBufferToByteArray(env, inputSignal, SIGNAL_LENGTH);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_neonintrinsicssamples_MainActivity_truncate(JNIEnv *env, jobject thiz, jboolean useNeon) {

    auto start = now();

#if HAVE_NEON
    if(useNeon)
        truncateNeon();
    else
#endif

    truncate();

    processingTime = usElapsedTime(start);

    return nativeBufferToByteArray(env, inputSignalTruncate, SIGNAL_LENGTH);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_neonintrinsicssamples_MainActivity_convolution(JNIEnv *env, jobject thiz, jboolean useNeon) {

    auto start = now();

#if HAVE_NEON
    if(useNeon)
        convolutionNeon();
    else
#endif

    convolution();

    processingTime = usElapsedTime(start);

    return nativeBufferToByteArray(env, inputSignalConvolution, SIGNAL_LENGTH);
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_example_neonintrinsicssamples_MainActivity_getProcessingTime(JNIEnv *env, jobject thiz) {
    return processingTime;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_neonintrinsicssamples_MainActivity_getSignalLength(JNIEnv *env, jobject thiz) {
    return SIGNAL_LENGTH;
}