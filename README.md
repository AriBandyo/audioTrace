# AudioTrace

Identifying copyrighted songs from short, noisy, and deliberately manipulated audio clips — the problem Shazam cannot solve.

---

## What is AudioTrace

AudioTrace is an audio fingerprinting system that detects when a copyrighted song has been used in a video, even after the audio has been deliberately pitch shifted, time stretched, or layered with noise to evade detection.

Platforms like TikTok, Instagram, and YouTube lose billions in music royalties every year because users modify audio to bypass existing detection systems. Shazam fails on distorted audio. YouTube Content ID fails on pitch shifted clips. No open, accessible system exists to catch this. AudioTrace is being built to solve exactly that.

---

## The Problem

```
Original song  →  Speed up by 15%  →  Shazam no match found
Original song  →  Pitch shift +3   →  YouTube Content ID no match found
Original song  →  Add background   →  Every classical system fails
               noise
```

Artists lose royalty credit. Labels lose licensing revenue. The manipulation is trivial to do and nearly impossible to catch with classical methods. This is an active, unsolved problem in the music industry.

---

## Architecture

### Phase 1 — Classical Pipeline (current)

```
Audio input
    ↓
Mel spectrogram       (TorchAudio)
    ↓
Peak extraction       (C++)
    ├── Density filter       → uniform coverage across time-frequency space
    └── Amplitude selection  → highest amplitude peak per region (noise robust)
    ↓
Constellation hashing  (Wang 2003 approach)
    ↓
Fingerprint database   (exact match lookup)
    ↓
Song match + timestamp
```

### Phase 2 — ML Layer for Manipulated Audio (roadmap)

```
Audio input
    ↓
Mel spectrogram + Peak extraction  (same as Phase 1)
    ↓
MERT backbone          (pretrained music foundation model)
    ↓
Projection head        (3 linear layers → 128d embedding)
    ↓
Faiss vector index     (nearest neighbour similarity search)
    ↓
Match result + confidence score
```

The ML layer is trained using contrastive learning — the same song with two different random manipulations is pushed toward the same point in embedding space. The model learns what features of a song survive distortion, which is exactly what is needed to catch TikTok style audio evasion.

---

## Why This Architecture

Classical peak hashing is fast, lightweight, and explainable. It handles clean audio perfectly. The ML layer sits on top of the same peak extraction and handles everything the classical method misses. This hybrid approach is the direction taken by PeakNetFP (Cortès-Sebastià et al., June 2025), currently considered state of the art for manipulation-robust fingerprinting.

Neither layer is throwaway work. Phase 1 is the input to Phase 2.

---

## Tech Stack

## Layer Technology

Peak extraction C++ with STL, multithreading
Spectrogram TorchAudio
ML backbone MERT (HuggingFace)
Training PyTorch + Lightly (NT-Xent contrastive loss)
Vector search Faiss
Augmentation Time stretch, pitch shift, noise, compression, filtering
Training data FMA (100K tracks), AudioSet, GTZAN

---

## Market Context

This is not an academic exercise. The problem has real commercial consequences

- Vobile acquired Pex (audio content ID) in April 2025
- Vobile launched AI Song Detector in November 2025 — but for detecting AI-generated music, not manipulation evasion. Different problem entirely
- Universal Music Group filed lawsuits against multiple platforms over unlicensed music use in 2024
- CISAC warned AI-generated and manipulated content could put 25% of musicians' income at risk by 2028
- Independent artists have no accessible tool to protect themselves

AudioTrace targets the gap nobody is filling an open, accessible system that catches manipulated copies of real songs on social platforms.

---

## Roadmap

- [x] Research — Wang 2003 paper, NeuralFP, PeakNetFP, MERT
- [x] Architecture design — two phase hybrid pipeline
- [ ] Phase 1 — spectrogram generation and peak extraction in C++
- [ ] Phase 1 — constellation hashing and database matching
- [ ] Phase 1 — baseline evaluation on clean audio
- [ ] Phase 2 — MERT integration and projection head
- [ ] Phase 2 — contrastive training pipeline with augmentations
- [ ] Phase 2 — Faiss embedding index and query matching
- [ ] Phase 2 — evaluation on manipulated audio benchmark
- [ ] API prototype for independent artist use case

---

## References

- Wang, A. (2003). An Industrial-Strength Audio Search Algorithm. ISMIR
- Chang et al. (2021). Neural Audio Fingerprint for High-Specific Audio Retrieval Based on Contrastive Learning. ICASSP
- Cortès-Sebastià et al. (2025). PeakNetFP Peak-based Neural Audio Fingerprinting. arXiv2506.21086
- Singh et al. (2025). Robust Neural Audio Fingerprinting using Music Foundation Models. arXiv2511.05399

---

## Author

Aritro Bandyopadhyay
[linkedin.cominaritro-bandyopadhyay](https://www.linkedin.com/in/aritro-bandyopadhyay/) · [github.comAriBandyo](https://github.com/AriBandyo)

---

AudioTrace is in active development. Star the repo to follow progress.
