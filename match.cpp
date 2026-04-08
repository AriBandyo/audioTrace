#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fftw3.h>
#include <sndfile.h>
#include <libpq-fe.h>

struct Fingerprint { int freq1, freq2, delta, t; };
struct Point { int window; double freq; };

std::vector<Fingerprint> loadFromDB(){
    std::vector<Fingerprint> db;
    PGconn* conn = PQconnectdb("host=localhost dbname=audiotrace user=aritr");
    if(PQstatus(conn) != CONNECTION_OK){
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn);
        return db;
    }

    PGresult* res = PQexec(conn, "SELECT freq1, freq2, delta, t FROM fingerprints;");
    if(PQresultStatus(res) != PGRES_TUPLES_OK){
        std::cerr << "Query failed: " << PQerrorMessage(conn) << "\n";
        PQclear(res);
        PQfinish(conn);
        return db;
    }

    int rows = PQntuples(res);
    for(int i = 0; i < rows; i++){
        int f1 = std::stoi(PQgetvalue(res, i, 0));
        int f2 = std::stoi(PQgetvalue(res, i, 1));
        int d  = std::stoi(PQgetvalue(res, i, 2));
        int t  = std::stoi(PQgetvalue(res, i, 3));
        db.push_back({f1, f2, d, t});
    }

    std::cout << "Loaded " << db.size() << " fingerprints from PostgreSQL\n";
    PQclear(res);
    PQfinish(conn);
    return db;
}

int main(){
    // load database from PostgreSQL
    auto db = loadFromDB();

    // load query wav
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

    // mono conversion
    std::vector<float> mono(info.frames);
    for(int i = 0; i < info.frames; i++){
        float s = 0;
        for(int c = 0; c < info.channels; c++)
            s += buf[i * info.channels + c];
        mono[i] = s / info.channels;
    }

    // FFT + constellation
    const int N = 1024;
    double* in = fftw_alloc_real(N);
    fftw_complex* out = fftw_alloc_complex(N/2 + 1);
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

    std::vector<Point> constellation;
    for(int start = 0; start + N <= (int)mono.size(); start += N){
        for(int i = 0; i < N; i++) in[i] = mono[start + i];
        fftw_execute(plan);

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

        int widx = start / N;
        for(int i = 0; i < 5 && i < (int)mags.size(); i++){
            int bin = mags[i].second;
            double freq = bin * info.samplerate / (double)N;
            constellation.push_back({widx, freq});
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    // generate query fingerprints
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

    // match
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