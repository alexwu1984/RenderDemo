#include "util.h"
#include <vector>
#include <ppl.h>
#include <iostream>
#include "harmonics.h"

inline void UpdateProgress(float progress)
{
	int barWidth = 70;

	std::cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0) << " %\r";
	std::cout.flush();
};

Harmonics::Harmonics(int degree)
	:m_degree(degree)
{
	size_t maxface = m_degree * m_degree;
	m_factorial.resize(maxface + 1);
	for (size_t i = 0; i<= maxface; ++i)
	{
		if (i == 0)
		{
			m_factorial[i] = 1;
		}
		else
		{
			m_factorial[i] = i * m_factorial[i - 1];
		}
	}
}

void Harmonics::Evaluate(const std::vector<Vertex>& vertices)
{
	int n = (m_degree + 1) * (m_degree + 1);
	coefs = std::vector<Vec3>(n, Vec3());
	for (const Vertex& v : vertices)
	{
		std::vector<float> Y = Basis(v.pos);
		for (int i = 0; i < n; ++i)
		{
			coefs[i] = coefs[i] + Y[i] * v.color;
		}
	}

	for (Vec3& coef : coefs)
	{
		coef = 4 * PI * coef / (float)vertices.size();
	}
}

Vec3 Harmonics::Render(const Vec3& pos)
{
	int n = (m_degree + 1) * (m_degree + 1);
	std::vector<float> Y = Basis(pos);
	Vec3 color;
	for (int i = 0; i < n; i++)
	{
		color = color + Y[i] * coefs[i];
	}
	return color;
}

std::array<cv::Mat, 6> Harmonics::RenderCubemap(int width, int height)
{
	std::array<cv::Mat, 6> imgs;
	for (int k = 0; k < 6; k++)
	{
		imgs[k] = cv::Mat(height, width, CV_32FC3);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				float u = (float)j / (width - 1);
				float v = 1.f - (float)i / (height - 1);
				Vec3 pos = CubeUV2XYZ({ k, u, v });
				Vec3 color = Render(pos);
				imgs[k].at<cv::Vec3f>(i, j) = { color.b, color.g, color.r };
			}
		}
	}
	return imgs;
}

std::vector<float> Harmonics::Render2(Vec3& normal, int width, int height)
{
	normal = Normalize(normal);
	std::vector<float> res(9);
	std::vector<float> count(9);
	for (int k = 0; k < 6; k++)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				float u = (float)j / (width - 1);
				float v = 1.f - (float)i / (height - 1);
				Vec3 w = (CubeUV2XYZ({ k, u, v }));
				std::vector<float> Y = Basis(w);
				w = Normalize(w);
				for (int index = 0; index < 9; index++)
				{
					float cos = w * normal;
					if (cos > 0)
						count[index]++;
					else
						continue;
					res[index] += cos * Y[index];
				}
			}
		}
	}
	for (int i = 0; i < 9; i++)
		res[i] = res[i] / count[i];
	return res;
}

std::array<cv::Mat, 18> Harmonics::RenderCubemap2(int width, int height)
{
	std::array<cv::Mat, 18> imgs;
	int index = 0;
	int cout = 0;
	for (int k = 0; k < 6; k++)
	{
		imgs[index] = cv::Mat(height, width, CV_32FC3);
		imgs[index + 1] = cv::Mat(height, width, CV_32FC3);
		imgs[index + 2] = cv::Mat(height, width, CV_32FC3);
		for (int i = 0; i < height; i++)
		{
			concurrency::parallel_for(0, width, [&](int j) {
				//for (int j = 0; j < width; j++)
				//{
				float u = (float)j / (width - 1);
				float v = 1.f - (float)i / (height - 1);
				Vec3 pos = CubeUV2XYZ({ k, u, v });
				std::vector<float> color = Render2(pos, 32, 32);
				imgs[index].at<cv::Vec3f>(i, j) = { color[0], color[1],color[2] };
				imgs[index + 1].at<cv::Vec3f>(i, j) = { color[3], color[4],color[5] };
				imgs[index + 2].at<cv::Vec3f>(i, j) = { color[6], color[7],color[8] };
				//}
			});
			++cout;
			UpdateProgress(cout / float(6 * height));
		}
		index += 3;
	}
	return imgs;
}

std::vector<float> Harmonics::Basis(const Vec3& pos)
{
	int n = (m_degree + 1) * (m_degree + 1);
	std::vector<float> Y(n);
	Vec3 normal = Normalize(pos);
	float x = normal.x;
	float y = normal.y;
	float z = normal.z;

	if (m_degree >= 0)
	{
		Y[0] = 1.f / 2.f * sqrt(1.f / PI);
	}
	if (m_degree >= 1)
	{
		Y[1] = sqrt(3.f / (4.f * PI)) * z;
		Y[2] = sqrt(3.f / (4.f * PI)) * y;
		Y[3] = sqrt(3.f / (4.f * PI)) * x;
	}
	if (m_degree >= 2)
	{
		Y[4] = 1.f / 2.f * sqrt(15.f / PI) * x * z;
		Y[5] = 1.f / 2.f * sqrt(15.f / PI) * z * y;
		Y[6] = 1.f / 4.f * sqrt(5.f / PI) * (-x * x - z * z + 2 * y * y);
		Y[7] = 1.f / 2.f * sqrt(15.f / PI) * y * x;
		Y[8] = 1.f / 4.f * sqrt(15.f / PI) * (x * x - z * z);
	}
	if (m_degree >= 3)
	{
		Y[9] = 1.f / 4.f * sqrt(35.f / (2.f * PI)) * (3 * x * x - z * z) * z;
		Y[10] = 1.f / 2.f * sqrt(105.f / PI) * x * z * y;
		Y[11] = 1.f / 4.f * sqrt(21.f / (2.f * PI)) * z * (4 * y * y - x * x - z * z);
		Y[12] = 1.f / 4.f * sqrt(7.f / PI) * y * (2 * y * y - 3 * x * x - 3 * z * z);
		Y[13] = 1.f / 4.f * sqrt(21.f / (2.f * PI)) * x * (4 * y * y - x * x - z * z);
		Y[14] = 1.f / 4.f * sqrt(105.f / PI) * (x * x - z * z) * y;
		Y[15] = 1.f / 4.f * sqrt(35.f / (2 * PI)) * (x * x - 3 * z * z) * x;
	}
	return Y;
}
