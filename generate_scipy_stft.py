import numpy as np
from scipy.signal import stft

# 1. 테스트 입력 신호 생성 (예: 10Hz + 20Hz sine wave)
fs = 125
t = np.arange(0, 2.0, 1/fs)
x = np.sin(2 * np.pi * 10 * t) + 0.5 * np.sin(2 * np.pi * 20 * t)

# 2. STFT 실행
f, t_stft, Zxx = stft(
    x,
    fs=fs,
    nperseg=62,
    noverlap=62 - 31,  # hop = 31
    nfft=62,
    padded=False,
    boundary=None
)

# 3. magnitude → dB 변환
magnitude = np.abs(Zxx)
db = 20 * np.log10(magnitude + 1e-10)  # 로그 0 방지용 작은 값 추가

# 4. 저장
np.savetxt("scipy_stft.csv", db, delimiter=",")
print("Saved STFT result to scipy_stft.csv")