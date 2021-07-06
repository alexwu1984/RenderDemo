#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include "util.h"
#include "cubemap.h"
#include "harmonics.h"

std::string CoefficientsString(const std::vector<Vec3>& coefs)
{
	std::ostringstream oss;
	for (const Vec3& c : coefs)
	{
		oss << c.r << "\t" << c.g << "\t" << c.b << std::endl;
	}
	return oss.str();
}

void CacluateCoeff(const std::array<std::string, 6>& img_files, const std::string& dir)
{
	bool write_rendered = true;
	int degree = 3;
	int samplenum = 1000000;

	std::string format = "jpg";
	std::string outdir = dir;
	std::cout << "reading cubemap ..." << std::endl;
	Cubemap cubemap(img_files);
	if (write_rendered)
	{
		std::string expandfile = outdir + "expand." + format;
		std::cout << "write expand cubemap image: " << expandfile << std::endl;
		cv::Mat expand = cubemap.GenExpandImage();
		cv::imwrite(expandfile, expand * 255);
	}

	Harmonics harmonics(degree);
	{
		std::cout << "sampling ..." << std::endl;
		auto verticies = cubemap.RandomSample(samplenum);
		harmonics.Evaluate(verticies);
	}

	std::cout << "---------- coefficients ----------" << std::endl;
	auto coefs = harmonics.getCoefficients();
	std::string coefstr = CoefficientsString(coefs);
	std::cout << coefstr;
	std::cout << "----------------------------------" << std::endl;

	std::ofstream coeffile(dir + "coefficients.txt");
	if (coeffile)
	{
		coeffile << coefstr;
		std::cout << "written " << dir + "coefficients.txt" << std::endl;
	}
	else
		std::cout << "write coefficients.txt failed" << std::endl;
}

void GenImage(const std::string& dir)
{
	int degree = 2;
	std::string format = "jpg";
	std::string outdir = dir;
	Harmonics harmonics(degree);
	auto shimgs = harmonics.RenderCubemap2(256, 256);
	for (int i = 0; i < 18; i++)
	{
		std::string outfile = outdir + "rendered_" + std::to_string(i) + "." + format;
		std::cout << "write rendered images: " << outfile << std::endl;
		cv::imwrite(outfile, shimgs[i] * 255);
	}
}

int main(int argc, char* argv[])
{
	std::string dir = "../../Resources/Textures/Skybox/";
	std::array<std::string, 6> faces = { "right", "left", "top", "bottom", "front", "back" };
	std::array<std::string, 6> img_files;
	std::string format = "jpg";
	for (int i = 0; i < 6; i++)
		img_files[i] = dir + faces[i] + "." + format;

	CacluateCoeff(img_files, dir);
	GenImage(dir);

	return 0;
}