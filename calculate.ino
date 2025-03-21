// digunakan untuk menghitung hari pasaran
int jumlahhari() { 
  //RtcDateTime now = Rtc.GetDateTime();
  int d = day();
  int m = month();
  int y = year();

  static const int hb[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  int ht = (y - 1970) * 365 - 1;
  int hs = hb[m - 1] + d;

  if (y % 4 == 0 && m > 2) hs++; // Tambahkan 1 hari jika tahun kabisat dan lewat Februari

  int kab = (y - 1969) / 4;  // Hitung langsung jumlah tahun kabisat

  return (ht + hs + kab);
}
