#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "benchmark.h"
#include <thread>
#include <mutex>
#include "threadpool.h"
using namespace std;

mutex mtx;
vector<string> read_env(string filename) {
	ifstream ifs;
	ifs.open(filename, ios::in);
	vector<string> buffer;
	string str;
	while (getline(ifs, str)) {
		buffer.push_back(str);
	}
	ifs.close();
	return buffer;
}
vector<string> read_flp(string filename) {
	ifstream ifs;
	ifs.open(filename,ios::in);
	vector<string> res;
	string str;
	while (getline(ifs, str)) {
		res.push_back(str);
	}
	ifs.close();
	return res;
}
void modify_env(vector<string> buf,string outname,float rho, float hs, float cst, float csb, float cb) {
	vector<string> loc = buf;
	buf[127] = string("1000 0.0 ")+to_string(127.0+hs).substr(0,6);
	buf[128] = string("127.0 ") + to_string(cst) + " 0.0 " + to_string(rho) + " 0.13 0.0";
	buf[129] = to_string(127.0 + hs).substr(0, 6) + " " +to_string(csb)+"/";
	buf[131] = to_string(127.0 + hs).substr(0, 6) + " " + to_string(cb) + " 0.0 1.75 0.15 0.0";
	// 输出修改的env 文件
	ofstream ofs;
	ofs.open(outname, ios::out);
	for (auto v : buf) {
		ofs << v << endl;
	}
	ofs.close();
	// 需要同步修改flp文件，并输出

}
void modify_flp(vector<string> buf, string outname) {
	vector<string> loc = buf;
	ofstream ofs;
	ofs.open(outname, ios::out);
	for (auto v : buf) {
		ofs << v << endl;
	}
	ofs.close();
}

int mtask(vector<string> envBuffer,vector<string> flpBuffer,int i) {
	string fname = string("saclant_") + to_string(i);
	modify_env(envBuffer, fname + ".env", 1.6 + float(i)/ 50, 2.7, 1530, 1570, 1600);
	modify_flp(flpBuffer, fname + ".flp");
	system(string("krakenc.exe " + fname).c_str());
	system(string("field.exe " + fname).c_str());

	remove(string(fname + ".flp").c_str());
	remove(string(fname + ".mod").c_str());
	remove(string(fname + ".prt").c_str());
	lock_guard<mutex> guard(mtx);
	//cout << i << endl;
	return i;
}
void stask(vector<string> envBuffer, vector<string> flpBuffer) {
	for (int i = 0; i < 50; i++) {
		string fname = string("saclant_") + to_string(i);
		modify_env(envBuffer, fname + ".env", 1.6 + i / 10, 2.7, 1530, 1570, 1600);
		modify_flp(flpBuffer, fname + ".flp");
		system(string("krakenc.exe " + fname).c_str());
		system(string("field.exe " + fname).c_str());

		remove(string(fname + ".flp").c_str());
		remove(string(fname + ".mod").c_str());
		remove(string(fname + ".prt").c_str());
		cout << i << endl;
	}
}
int main() {
	cout << "hello world" << endl;
	auto envBuffer = read_env("saclant.env");
	auto flpBuffer = read_flp("saclant.flp");
	char cmd1[50];
	// 单线程处理
	BenchMark *b1 = new BenchMark("main");
	stask(envBuffer, flpBuffer);
	delete b1;
	// 多线程加速
	BenchMark *b2 = new BenchMark("main");

	
	threadpool pool{ 50 };
	std::vector< std::future<int> > res;
	for (int i = 0; i < 50; i++) {
		res.emplace_back(
			pool.commit(mtask, envBuffer, flpBuffer, i)
		);
	}
	cout << " =======  commit all ========= " << endl;
	for (auto &&v: res) {
		cout << v.get()<<" done" << endl;
	}
	//stask(envBuffer, flpBuffer);
	//thread ths[20];
	//for (int i = 0; i < 20; i++) {
	//	ths[i]=thread(mtask, envBuffer, flpBuffer, i);
	//}
	//for (int i = 0; i < 20; i++) {
	//	ths[i].join();
	//}
	cout << "done!" << endl;
	delete b2;

	system("pause");
}