#define _USE_MATH_DEFINES

#include <cmath>
#include <complex>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

typedef complex<double> dcomplex;
typedef long long ll;

string multiply(const string& a, const string& b);
vector<ll> convolution(vector<dcomplex>& a, vector<dcomplex>& b);
void dft(vector<dcomplex>& a);
void idft(vector<dcomplex>& a);
void fft(vector<dcomplex>& a, bool inv);

int main() {
  ios_base::sync_with_stdio(false);
  cin.tie(NULL);

  string a, b;
  cin >> a >> b;
  cout << multiply(a, b) << endl;

  return 0;
}

string multiply(const string& a, const string& b) {
  if (a == "0" || b == "0") return "0";

  vector<dcomplex> aa(a.size(), 0);
  for (int i = 0; i < (int)a.size(); i++) aa[i] = a[i] - '0';

  vector<dcomplex> bb(b.size(), 0);
  for (int i = 0; i < (int)b.size(); i++) bb[i] = b[i] - '0';

  vector<ll> cc = convolution(aa, bb);
  int n = cc.size();

  for (int i = n - 1; i > 0; i--) {
    cc[i - 1] += cc[i] / 10;
    cc[i] = cc[i] % 10;
  }

  stringstream ss;
  for (ll digit : cc) ss << digit;
  string c = ss.str();
  return c.substr(c.find_first_not_of('0'));
}

vector<ll> convolution(vector<dcomplex>& a, vector<dcomplex>& b) {
  int asize = a.size();
  int bsize = b.size();

  int n = 1;
  while (n < asize + bsize) n <<= 1;

  vector<dcomplex> aa(n, 0);
  for (int i = n - asize; i < n; i++) aa[i] = a[i - (n - asize)];

  vector<dcomplex> bb(n, 0);
  for (int i = n - bsize; i < n; i++) bb[i] = b[i - (n - bsize)];

  dft(aa);
  dft(bb);

  for (int i = 0; i < n; i++) aa[i] *= bb[i];
  idft(aa);

  vector<ll> c(n, 0);
  for (int i = 0; i < n; i++) c[i] = round(aa[i].real());
  return c;
}

void dft(vector<dcomplex>& a) { fft(a, false); }

void idft(vector<dcomplex>& a) { fft(a, true); }

void fft(vector<dcomplex>& a, bool inv) {
  int n = a.size();
  while (n > 1) {
    if (n & 1) throw "a must be a power of 2";
    n >>= 1;
  }
  n = a.size();

  for (int src = 0; src < n; src++) {
    int dst = 0;
    int tmp = src;

    for (int _ = 1; _ < n; _ <<= 1) {
      dst = (dst << 1) | (tmp & 1);
      tmp >>= 1;
    }

    if (src < dst) swap(a[src], a[dst]);
  }

  // NOTE: 계수를 정방향으로 유지하면서 구조가 복잡해짐
  for (int portion_size = 2; portion_size <= n; portion_size <<= 1) {
    double w_unit = 2 * M_PI / portion_size;
    if (inv) w_unit *= -1;

    for (int odd_idx = 0; odd_idx < n; odd_idx += portion_size) {
      dcomplex w_i = 1;
      int even_idx = odd_idx | (portion_size >> 1);

      for (int f_i = (portion_size >> 1) - 1; f_i >= 0; f_i--) {
        dcomplex cache_even = a[even_idx | f_i];
        dcomplex cache_odd = a[odd_idx | f_i];

        a[odd_idx | f_i] = cache_even - w_i * cache_odd;
        a[even_idx | f_i] = cache_even + w_i * cache_odd;

        w_i *= dcomplex(cos(w_unit), sin(w_unit));
      }
    }
  }

  if (inv)
    for (int i = 0; i < n; i++) a[i] /= n;
}
