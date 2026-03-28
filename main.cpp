#include <iostream>
#include <vector>
#include <sndfile.h>
// The addition of FFT
#include <fftw3.h>
#include<cmath>
#include <algorithm>
#include<map>
#include<fstream>

struct Fingerprint{
    int freq1;
    int freq2;
    int delta;
    int time; // this used when hash occurs in the song

};

struct Point{
        int window;
        double freq;
    };

int main () {

// OPEN THE WAV FILES
const char* path = "test.wav"; // points the varaible "path" to the place were test.wav is located.
//storing the format of the sound by using SF_INFO which is a struct part of library libsnfile
SF_INFO info {};
SNDFILE* f = sf_open(path, SFM_READ, &info);
if(!f) {
    std::cerr << "Could not open file\n";

    return 1;
}
// READ ALL THE NUMBERS 
// buf is a temp storage that stores all the presure waves into it [L0, R0, L1, R1, L2, R2, L3, R3 ...]
std::vector<float> buf(info.frames * info.channels);
std::vector<Fingerprint> fingerprints;
int fan_out = 20;
sf_readf_float(f,buf.data(),info.frames);
sf_close(f);
// CONVERT THESE NUMBERS INTO MONO
std::vector<float> mono(info.frames); // the size of the mono is same as the that of the frames because in a very simple terms we will be find the avg(explanation is given in the read me section) so we do not need any vector bigger thn frames.
for (int i = 0; i < info.frames ; i++){
    float s = 0;
    for(int c = 0 ; c <info.channels ; c++){
        s += buf[i*info.channels + c];// this calulation is done taking into the conideration of edge case that the fact that c++ reads any garbage value at that memory point.
    }
    mono[i] = s /info.channels; // The L+R / number of channels calulation for each channel that you have specially taken care for mono sounds.
}
// PRINT THE INFO
std::cout<<"Sample Rate :" <<info.samplerate<<"\n";
std::cout<<"Channels    :" <<info.channels<<"\n";
std::cout<<"Frames Read :" << info.frames<<"\n";
for ( int i = 0 ; i< 5 ; i++){
    std::cout<<mono[i]<<" ";
}
std::cout<<"\n";

//this is where my FFT chunck goes in
//first we define the size of the window.
const int N = 1024;// this denotes the number of samples per windows.

//setting up FFTW
double* in = fftw_alloc_real(N); //allocates a block of memoery to hold N real numbers.

fftw_complex* out = fftw_alloc_complex(N/2 +1);

fftw_plan plan = fftw_plan_dft_r2c_1d(N, in , out , FFTW_ESTIMATE);

std::vector<Point> constellation;


//looping through mono with the window created.
for ( int start = 0 ; start + N <= mono.size() ; start += N){
    for( int i = 0; i< N ; i++){
        in[i] = mono[start +i];
    }
    //run cmd
    fftw_execute(plan);
    std::vector <std::pair<double,int>> mags;
    
   
    //magnituted of each frequency 
    //This also the window loop.
    for(int i = 0; i< N/2 + 1 ; i++){
        double real = out[i][0];
        double imag = out[i][1];
        double magnitude = sqrt(real*real + imag*imag);

        double freq = i * info.samplerate / (double)N;
        if ( freq < 300.0 || freq > 3000.0) continue;
        mags.push_back({magnitude,i});


    }   
    std::sort(mags.begin(),mags.end(), [](auto& a, auto& b){
        return a.first > b.first ;

    });
   

    //gettign the top 5 peaks
    int windows_index = start / N;
    for (int i = 0 ; i< 5 && i< (int)mags.size() ; i++){
        int bin = mags[i].second;
        double freq = bin * info.samplerate /(double)N;
        constellation.push_back({windows_index, freq});

    }
    
}
std::cout <<"Total constilation points :"<< constellation.size()<< "\n";

    for (int i = 0;  i<(int)constellation.size(); i++)
    {
        for(int j = i+1 ; j < i+fan_out && j< (int)constellation.size();j++){
            int freq1 = (int)constellation[i].freq;
            int freq2 = (int)constellation[j].freq;
            int delta = constellation[j].window - constellation[i].window;
            int t     = constellation[i].window;

            if(delta < 0 )continue;

            fingerprints.push_back({freq1, freq2 ,delta, t});

        }
    } 
    std::cout << "Total fingerprints: " << fingerprints.size() << "\n";
    for(int i = 0; i < 5 && i < (int)fingerprints.size(); i++){
    std::cout << "Hash: (" << fingerprints[i].freq1 
              << ", "      << fingerprints[i].freq2
              << ", dt="   << fingerprints[i].delta
              << ") at t=" << fingerprints[i].time << "\n";
}
std::ofstream file("fingerprints.db");
if(!file){
    std::cerr << "Could not open file for writing\n";
    return 1;
}

for(auto& fp : fingerprints){
    file << fp.freq1 << " " 
         << fp.freq2 << " " 
         << fp.delta << " " 
         << fp.time  << "\n";
}

file.close();
std::cout << "Fingerprints saved to fingerprints.db\n";

//clean up functions
fftw_destroy_plan(plan);
fftw_free(in); //use to free the allocated memory,
fftw_free(out);//use to free the allocated memory,

return 0;


}
