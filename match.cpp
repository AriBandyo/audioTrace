// This the file that will match the query or the trimmed audio to the finger print db
#include<iostream>
#include<vector>
#include<fstream>
#include<sndfile.h>
#include<fftw3.h>
#include<cmath>
#include<algorithm>
#include<map>

struct Fingerprint{
    int freq1;
    int freq2;
    int delta;
    int t;
};
std::vector<Fingerprint> loadDB (const std::string& path){
    std::vector<Fingerprint> db;
    std::ifstream file(path);
    int f1,f2,d,t;
    while(file >> f1>> f2>>d>>t){
        db.push_back({f1,f2,d,t});
    }
    std::cout<<"Loaded"<<db.size()<<"fingerprints from DB\n";
    return db;
}

int main(){
    auto db = loadDB("fingerprints.db");  // compiler sees loadDB returns std::vector<Fingerprint> and sets that as the type
    // LOAD QUERY WAV
const char* queryPath = "query.wav";
SF_INFO info {};
SNDFILE* f = sf_open(queryPath, SFM_READ, &info);
if(!f){
    std::cerr << "Could not open query.wav\n";
    return 1;
}

std::vector<float> buf(info.frames * info.channels);
sf_readf_float(f, buf.data(), info.frames);
sf_close(f);

// MONO CONVERSION
std::vector<float> mono(info.frames);
for(int i = 0; i < info.frames; i++){
    float s = 0;
    for(int c = 0; c < info.channels; c++)
        s += buf[i * info.channels + c];
    mono[i] = s / info.channels;
}

// FFT + PEAK FINDING
const int N = 1024;
double* in = fftw_alloc_real(N);
fftw_complex* out = fftw_alloc_complex(N/2 + 1);
fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

struct Point { int window; double freq; };
std::vector<Point> constellation;
//looping through mono with the window created.
for(int start = 0; start + N <= (int)mono.size(); start += N){
    for(int i = 0; i < N; i++) in[i] = mono[start + i];
    fftw_execute(plan);//
   //magnituted of each frequency 
    //This also the window loop.
    std::vector<std::pair<double,int>> mags;
    for(int i = 0; i < N/2 + 1; i++){
        double real = out[i][0], imag = out[i][1];
        double mag = sqrt(real*real + imag*imag);
        double freq = i * info.samplerate / (double)N;
        if(freq < 300.0 || freq > 3000.0) continue;
        mags.push_back({mag, i});
    }
    std::sort(mags.begin(), mags.end(), [](auto& a, auto& b){
        return a.first > b.first;
    });
    //gettign the top 5 peaks
    int widx = start / N;
    for(int i = 0; i < 5 && i < (int)mags.size(); i++){
        int bin = mags[i].second;
        double freq = bin * info.samplerate / (double)N;
        constellation.push_back({widx, freq});
    }
}
//clean up 
fftw_destroy_plan(plan);
fftw_free(in);
fftw_free(out);

// GENERATE QUERY FINGERPRINTS
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
// MATCHING
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

if(matches > 100){
    std::cout << "Song IDENTIFIED!\n";
} else {
    std::cout << "No match found\n";
}
}