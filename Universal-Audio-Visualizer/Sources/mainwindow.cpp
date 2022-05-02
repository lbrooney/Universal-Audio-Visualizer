#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define AUDIBLE_RANGE_START 20
#define AUDIBLE_RANGE_END   20000
#define NUM_SAMPLES 96000
#define SAMPLE_FREQ 48000

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for (int i = 0; i < NUM_SAMPLES; i ++)
    {
        mIndices.append((double)i);
        mSamples.append(0);
    }

    double freqStep = (double)SAMPLE_FREQ / (double)NUM_SAMPLES;
    double f = AUDIBLE_RANGE_START;
    while (f < AUDIBLE_RANGE_END)
    {
        mFftIndices.append(f);
        f += freqStep;
    }

    mFftIn  = fftw_alloc_real(NUM_SAMPLES);
    mFftOut = fftw_alloc_real(NUM_SAMPLES);
    mFftPlan = fftw_plan_r2r_1d(NUM_SAMPLES, mFftIn, mFftOut, FFTW_R2HC,FFTW_ESTIMATE);
}

MainWindow::~MainWindow()
{
    delete ui;

    fftw_free(mFftIn);
    fftw_free(mFftOut);
    fftw_destroy_plan(mFftPlan);
}


