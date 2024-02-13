#include <iostream>
#include <windows.h>
#include <string>
#include <concepts>
#include <utility>
#include <array>
#include <functional>
#include <algorithm>
#include <vector>

#define OCCUPIED false
template <std::size_t hash_size, typename T = const char*>
requires std::is_same_v<T, const char*>
class hash_table
{
	mutable void* hash_table_buffer = nullptr;
	using hash_t = signed long;
	using int64 = unsigned long long;
	mutable std::vector<int> dynamic_valid_indexes;
public:

	hash_table()
	{
		this->hash_table_buffer = VirtualAlloc(nullptr, hash_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		std::memset(this->hash_table_buffer, 0, hash_size);
	}

	hash_t get_hash(const T value) const noexcept(true)
	{
		int seed = 0x90;
		static int call_times = 0;
		std::array< hash_t, 50 > static_arr{ };
		try
		{
			if (call_times < hash_size)
			{
				std::generate(static_arr.begin(), static_arr.end(), [&]() -> hash_t {
					for (auto& current_c : std::string(value)) {
						seed += current_c ^ (seed & (current_c & 0xF) * seed) * seed;
					}
					return seed ^ hash_size;
					});

				std::for_each(static_arr.begin(), static_arr.end(), [&](int arg) {seed += arg; });
				call_times++;
				return seed % hash_size;
			}
			else {
				throw std::runtime_error("Hash table is full!!");
			}
		}
		catch (std::runtime_error& rt_error) {
			MessageBoxA(NULL, std::string("Error: ").append(rt_error.what()).c_str(), "Error", MB_ICONERROR | MB_ICONEXCLAMATION);
			return -1;
		}
	}

	std::pair<T, hash_t> add(const T value) const noexcept(true)
	{
		hash_t hash_index = this->get_hash(value);

		if (is_space(hash_index, sizeof(value)))
		{
			strcpy_s(reinterpret_cast<char*>(reinterpret_cast<int64>(this->hash_table_buffer) + hash_index), sizeof(value), value);
			this->dynamic_valid_indexes.push_back(hash_index);
			return std::make_pair(value, hash_index);
		}

		return std::make_pair(value, OCCUPIED);
	}

	T operator[](hash_t hash_index)
	{
		if (not std::count_if(this->dynamic_valid_indexes.begin(), this->dynamic_valid_indexes.end(),
			std::bind(std::equal_to{}, std::placeholders::_1, hash_index))) return "Undefined";

		return reinterpret_cast<const char*>(reinterpret_cast<int64>(this->hash_table_buffer) + hash_index);
	}

	template < typename ret > void loop_hash_table(std::function< ret( const char* current_name ) > loop_callback) noexcept(true)
	{
		std::sort(this->dynamic_valid_indexes.begin(), this->dynamic_valid_indexes.end());

		for (auto& idx : this->dynamic_valid_indexes)
		{
			loop_callback((*this)[idx]);
		}
	}

	~hash_table()
	{
		if (this->hash_table_buffer) VirtualFree(this->hash_table_buffer, NULL, MEM_RELEASE);
	}

private:

	// here we check if there is a range of contiguous unused bytes so we can store our string
	bool is_space(const int hash_index, std::size_t obj_size) const noexcept(true)
	{
		int64 potential_start = reinterpret_cast<int64>(this->hash_table_buffer) + hash_index;
		auto& p_auto = potential_start, range_end = potential_start + obj_size;

		for (; p_auto < range_end + 1; p_auto++)
		{
			auto current_byte = *reinterpret_cast<int*>(p_auto);

			if (not current_byte) continue;
			else break;
		}

		if (p_auto == range_end || p_auto > range_end) {
			return true;
		} return OCCUPIED;
	}
};
int main()
{

	hash_table<0x1000> hash_container;


	auto hash_member = hash_container.add("Kyle");
	std::cout << hash_container[hash_member.second] << std::endl;

	system("pause>0");
}
