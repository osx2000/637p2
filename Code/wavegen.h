#ifndef WAVEGEN_H
#define WAVEGEN_H


class WaveGen {
 public:
  WaveGen();
  WaveGen();
  ~WaveGen();
  int envelope(double);
  void interpolate();


 private:
  float *actual;
  float *working;
  int   *brkpts;
  int   brk;
  int   frames;
};

#endif
