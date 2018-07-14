// The default random module seems to be broken currently
class Random {
  construct new(seed) {
    _seed = (seed % 2147483646).abs % 2147483647
    if (_seed <= 0) {
      _seed = _seed + 2147483646
    }
  }

  next() {
   _seed = _seed * 16807 % 2147483647
   return _seed
  }

  int(n) {
    return next() % n
  }

  float() {
    return (next() - 1) / 2147483646
  }
}
