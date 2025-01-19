#pragma once

#ifdef ANCIENT_EXPORTS 
#define ANCIENT_API __declspec(dllexport)
#else 
#define ANCIENT_API __declspec(dllimport)
#endif

#include <random>

constexpr float scr_width{ 1000.0f };
constexpr float scr_height{ 700.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 650.0f };

constexpr uint8_t no_flag{ 0b00000000 };
constexpr uint8_t evil1_flag{ 0b00000001 };
constexpr uint8_t evil2_flag{ 0b00000010 };
constexpr uint8_t evil3_flag{ 0b00000100 };
constexpr uint8_t evil_med_flag{ 0b00001000 };
constexpr uint8_t hero_flag{ 0b00010000 };
constexpr uint8_t shot_flag{ 0b00100000 };

enum class dirs { left = 0, right = 1, up = 2, down = 3, stop = 4 };

namespace dll
{

	class ANCIENT_API RANDiT
	{
		private:
			std::random_device rd{};
			std::seed_seq* sq{ nullptr };
			std::mt19937* twister{ nullptr };

		public:
			RANDiT();
			~RANDiT();

			int operator() (int min, int max);
	};

	struct ANCIENT_API PROT_POINT
	{
		float x{ 0 };
		float y{};
	};

	class ANCIENT_API PROTON
	{
		protected:
			float width{};
			float height{};

		public:
			PROT_POINT start{ 0,0 };
			PROT_POINT end{ 0,0 };

			PROTON(float _sx = 0, float _sy = 0, float _width = 1.0f, float _height = 1.0f);
			virtual ~PROTON();

			float GetWidth() const;
			float GetHeight() const;

			void SetEdges();

			void SetWidth(float _new_width);
			void SetHeight(float _new_height);
			void NewDims(float _new_width, float _new_height);
	};

	class ANCIENT_API PROTON_MESH
	{
		private:
			PROTON* m_ptr{ nullptr };
			size_t max_size{ 0 };
			size_t next_position{ 0 };

		public:
			PROTON_MESH(size_t num_of_elements);
			~PROTON_MESH();

			void push_back(PROTON& element);
			void remove(size_t index);
			size_t capacity() const;
			size_t size() const;

			PROTON& sort(PROT_POINT target);

			PROTON& operator[](size_t index);
			void operator()(size_t index, PROTON& element);
	};

	class ANCIENT_API CREATURE :public PROTON
	{
		protected:
			float my_speed{ 1.0f };

			float move_x{ 0 };
			float move_y{ 0 };
			float move_ex{ 0 };
			float move_ey{ 0 };
			float slope{ 0 };
			float intercept{ 0 };

			bool hor_line = false;
			bool vert_line = false;

			int strenght = 0;
			int attack_delay{ 0 };

			int frame{ 0 };
			int max_frames{ 0 };
			int frame_delay{ 0 };

			void SetPathInfo(float to_where_x, float to_where_y);

			uint8_t my_type{ 0 };

			RANDiT class_rand{};

		public:
			int lifes = 0;
			dirs dir = dirs::right;

			CREATURE(float first_x, float first_y, uint8_t type);
			CREATURE(float first_x, float first_y, float end_x, float end_y);

			virtual ~CREATURE();

			uint8_t GetType() const;
			void SetType(uint8_t to_what);

			int GetFrame();
			int Attack();

			float Distance(PROT_POINT my_point, PROT_POINT dest_point);

			virtual bool Move(float gear, float targ_x, float targ_y) = 0;
			virtual PROT_POINT AINextMove(PROTON_MESH& army, float hero_x, float hero_y) = 0;
			virtual void Release() = 0;
	};

	typedef CREATURE* Object;

	class ANCIENT_API EVIL :public CREATURE
	{
		private:

			EVIL(uint8_t type, float where_x, float where_y);

		public:
			friend ANCIENT_API Object CreatureFactory(float sx, float sy, uint8_t what_type);

			bool Move(float gear, float targ_x, float targ_y) override;
			PROT_POINT AINextMove(PROTON_MESH& army, float hero_x, float hero_y) override;
			void Release() override;
	};

	class ANCIENT_API HERO :public CREATURE
	{
	private:

		HERO(float where_x, float where_y);

	public:
		friend ANCIENT_API Object CreatureFactory(float sx, float sy, uint8_t what_type);

		bool Move(float gear, float targ_x, float targ_y) override;
		PROT_POINT AINextMove(PROTON_MESH& army, float hero_x, float hero_y) override;
		void Release() override;
	};

	class ANCIENT_API SHOT :public CREATURE
	{
	private:

		SHOT(float where_x, float where_y, float to_where_x, float to_where_y);

	public:
		friend ANCIENT_API Object ShotFactory(float sx, float sy, float ex, float ey);

		bool Move(float gear, float targ_x, float targ_y) override;
		PROT_POINT AINextMove(PROTON_MESH& army, float hero_x, float hero_y) override;
		void Release() override;
	};

	ANCIENT_API Object CreatureFactory(float sx, float sy, uint8_t what_type);
	ANCIENT_API Object ShotFactory(float sx, float sy, float ex, float ey);
}