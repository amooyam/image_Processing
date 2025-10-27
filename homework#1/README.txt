# README

이 폴더는 영상처리 실습 #1을 위한 코드입니다.  
`main.c` 파일을 실행하면 다음과 같은 동작을 수행합니다.

## 실행 방법
1. Visual Studio Code(VSC) 또는 Visual Studio(VS)에서 프로젝트를 엽니다.  
2. `main.c` 파일을 빌드하고 실행합니다.  
3. 실행이 완료되면 결과 파일이 저장됩니다.

## 출력 파일 경로
- `images/` 폴더: 영상 결과 파일이 저장됩니다.  
    `rMinus.raw`, `equalized_hist.raw`, `matched_hist.raw`

- `histograms/` 폴더: 각 영상의 히스토그램 결과 파일이 저장됩니다.  
    `rminus_hist_256x256.raw`, `equalized_hist_256x256.raw`, `matched_hist_256x256.raw`

## 주의 사항
- 압축 해제시 입력 영상 파일(`lena_512x512.raw`)은 `images/` 폴더에 이미 존재합니다. 
- 프로그램 실행을 위한 폴더 구조는 다음과 같습니다.

프로젝트 폴더/
├─ src/
│   ├─ main.c
│   ├─ picOps.c
│   ├─ picOps.h
│   ├─ picIO.h
│   └─ common.h
├─ images/
│   └─ lena_512x512.raw   ← 입력 영상 파일
└─ histograms/