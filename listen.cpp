#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <portaudio.h>
#include <fftw3.h>

#define SAMPLE_RATE    44100
#define FRAMES_PER_BUF 1024
#define SECONDS        5

struct Fingerprint { int freq1, freq2, delta, t; };
struct Point { int window; double freq; };

struct AudioData {
    std::vector<float> samples;
    int maxFrames;
};

static int audioCallback(
    const void* input, void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    AudioData* data = (AudioData*)userData;
    const float* in = (const float*)input;
    if(!in) return paContinue;
    for(unsigned long i = 0; i < frameCount; i++){
        if((int)data->samples.size() < data->maxFrames)
            data->samples.push_back(in[i]);
    }
    if((int)data->samples.size() >= data->maxFrames)
        return paComplete;
    return paContinue;
}

std::vector<Fingerprint> loadDB(const std::string& path){
    std::vector<Fingerprint> db;
    std::ifstream file(path);
    int f1, f2, d, t;
    while(file >> f1 >> f2 >> d >> t)
        db.push_back({f1, f2, d, t});
    std::cout << "Loaded " << db.size() << " fingerprints from DB\n";
    return db;
}

int main(){
    // load database
    auto db = loadDB("fingerprints.db");

    // record from mic
    Pa_Initialize();
    AudioData data;
    data.maxFrames = SAMPLE_RATE * SECONDS;

    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 1, 0, paFloat32,
                         SAMPLE_RATE, FRAMES_PER_BUF,
                         audioCallback, &data);

    std::cout << "Recording for " << SECONDS << " seconds... play music!\n";
    Pa_StartStream(stream);
    while(Pa_IsStreamActive(stream) == 1) Pa_Sleep(100);
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "Recorded " << data.samples.size() << " samples\n";

    // FFT + constellation
    const int N = 1024;
    double* in = fftw_alloc_real(N);
    fftw_complex* out = fftw_alloc_complex(N/2 + 1);
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

    std::vector<Point> constellation;

    for(int start = 0; start + N <= (int)data.samples.size(); start += N){
        for(int i = 0; i < N; i++) in[i] = data.samples[start + i];
        fftw_execute(plan);

        std::vector<std::pair<double,int>> mags;
        for(int i = 0; i < N/2 + 1; i++){
            double real = out[i][0], imag = out[i][1];
            double mag = sqrt(real*real + imag*imag);
            double freq = i * SAMPLE_RATE / (double)N;
            if(freq < 300.0 || freq > 3000.0) continue;
            mags.push_back({mag, i});
        }
        std::sort(mags.begin(), mags.end(), [](auto& a, auto& b){
            return a.first > b.first;
        });

        int widx = start / N;
        for(int i = 0; i < 5 && i < (int)mags.size(); i++){
            int bin = mags[i].second;
            double freq = bin * SAMPLE_RATE / (double)N;
            constellation.push_back({widx, freq});
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    std::cout << "Constellation points: " << constellation.size() << "\n";

    // generate fingerprints from mic audio
    std::vector<Fingerprint> query_fps;
    int fan_out = 20;
    for(int i = 0; i < (int)constellation.size(); i++){
        for(int j = i+1; j < i+fan_out && j < (int)constellation.size(); j++){
            int freq1 = (int)constellation[i].freq;
            int freq2 = (int)constellation[j].freq;
            int delta = constellation[j].window - constellation[i].window;
            int t     = constellation[i].window;
            query_fps.push_back({freq1, freq2, delta, t});
        }
    }
    std::cout << "Query fingerprints: " << query_fps.size() << "\n";

    // match against database
    int matches = 0;
    for(auto& qfp : query_fps){
        for(auto& dbfp : db){
            if(qfp.freq1 == dbfp.freq1 &&
               qfp.freq2 == dbfp.freq2 &&
               qfp.delta == dbfp.delta){
                matches++;
                break;
            }
        }
    }

    std::cout << "Matches found: " << matches << "\n";
    std::cout << "Match score: " << (100.0 * matches / query_fps.size()) << "%\n";

    if(matches > 100)
        std::cout << "Song IDENTIFIED!\n";
    else
        std::cout << "No match found\n";

    return 0;
}