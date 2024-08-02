#pragma once

class Noise 
{
public:
  virtual ~Noise() {}
  virtual float compute(float x, float y) = 0;
};