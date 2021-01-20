#include <chrono>
#include <iostream>

void clobber()
{
	asm volatile("" : : : "memory");
}

static void escape(void* p)
{
	asm volatile("" : : "g"(p) : "memory");
}

class Timer {
	std::chrono::high_resolution_clock::time_point m_start;

public:
	Timer()
	{
		m_start = std::chrono::high_resolution_clock::now();
	};
	~Timer()
	{
		const auto end = std::chrono::high_resolution_clock::now();
		const auto duration
			= std::chrono::duration_cast<std::chrono::milliseconds>(
				  end - m_start)
				  .count();
		std::cout << duration << std::endl;
	};
};

// 1024 -> 4MiB
constexpr uintptr_t size = 1024 / 2;
using float_array = float[size][size];

int main()
{
	float_array& A = *reinterpret_cast<float_array*>(new float_array{});
	float_array& B = *reinterpret_cast<float_array*>(new float_array{});
	float_array& C = *reinterpret_cast<float_array*>(new float_array{});

	for (auto i = 0; i < size; i++) {
		for (auto j = 0; j < size; j++) {
			A[i][j] = static_cast<float>(i * 1024 + j);
			B[i][j] = static_cast<float>(j * 1024 + i);
			C[i][j] = 0;
		}
	}
	clobber();
	{
		Timer timer{};
		std::cout << "ijk: ";
		////////////////////////////////////////
		for (auto i = 0; i < size; i++) {
			for (auto j = 0; j < size; j++) {
				for (auto k = 0; k < size; k++) {
					C[i][j] += A[i][k] * B[k][j];
					escape(&(C[i][j]));
				}
			}
		}
		////////////////////////////////////////
	}
	clobber();
	for (auto i = 0; i < size; i++) {
		for (auto j = 0; j < size; j++) {
			C[i][j] = 0;
		}
	}
	clobber();
	{
		Timer timer{};
		std::cout << "ikj: ";
		////////////////////////////////////////
		for (auto i = 0; i < size; i++) {
			for (auto k = 0; k < size; k++) {
				auto tmp = A[i][k];
				for (auto j = 0; j < size; j++) {
					C[i][j] += tmp * B[k][j];
					escape(&C[i][j]);
				}
			}
		}
		////////////////////////////////////////
	}
	clobber();
	for (auto i = 0; i < size; i++) {
		for (auto j = 0; j < size; j++) {
			C[i][j] = 0;
		}
	}
	clobber();
	{
		Timer timer{};
		std::cout << "transpose: ";
		float_array& BT = *reinterpret_cast<float_array*>(new float_array);
		for (auto i = 0; i < size; i++) {
			for (auto j = 0; j < size; j++) {
				BT[i][j] = B[j][i];
			}
		}
		////////////////////////////////////////
		for (auto i = 0; i < size; i++) {
			for (auto j = 0; j < size; j++) {
				for (auto k = 0; k < size; k++) {
					C[i][j] += A[i][k] * BT[j][k];
					escape(&C[i][j]);
				}
			}
		}
		////////////////////////////////////////
	}
	clobber();
}
