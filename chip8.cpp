#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>
#include <iostream>
#include <SDL2/SDL.h>


const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int START_ADDRESS = 0x200;

uint8_t fontset[FONTSET_SIZE] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

class Chip8
{
public:
    using Chip8Func = void (Chip8::*)();

    uint8_t  gpr[16]{};
    uint8_t  memory[4096]{};
    uint16_t ir{};
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t  sp{};
    uint8_t  delay_Timer{};
    uint8_t  sound_Timer{};
    uint8_t  keypad[16]{};
    uint32_t display[64 * 32]{};
    uint16_t opcode{};

    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

    Chip8Func table[0x10]{};
    Chip8Func table0[0x10]{};
    Chip8Func table8[0x10]{};
    Chip8Func tableE[0x10]{};
    Chip8Func tableF[0x100]{};

    Chip8();

    void Cycle();
    void LoadROM(const char* filename); 
    void LoadFontset();

    void Table0() { ((*this).*(table0[opcode & 0x000Fu]))(); }
    void Table8() { ((*this).*(table8[opcode & 0x000Fu]))(); }
    void TableE() { ((*this).*(tableE[opcode & 0x000Fu]))(); }
    void TableF() { ((*this).*(tableF[opcode & 0x00FFu]))(); }
    void OP_NULL() {}

    void OP_00E0(); void OP_00EE();
    void OP_1NNN(); void OP_2NNN(); void OP_3XNN(); void OP_4XNN();
    void OP_5XY0(); void OP_6XNN(); void OP_7XNN();
    void OP_8XY0(); void OP_8XY1(); void OP_8XY2(); void OP_8XY3();
    void OP_8XY4(); void OP_8XY5(); void OP_8XY6(); void OP_8XY7();
    void OP_8XYE();
    void OP_9XY0(); void OP_ANNN(); void OP_BNNN(); void OP_CXNN();
    void OP_DXYN();
    void OP_EX9E(); void OP_EXA1();
    void OP_FX07(); void OP_FX0A(); void OP_FX15(); void OP_FX18();
    void OP_FX1E(); void OP_FX29(); void OP_FX33();
    void OP_FX55(); void OP_FX65();
};
Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()),
      randByte(0, 255)
{
    pc = START_ADDRESS;
    LoadFontset();

    for (auto &f : table)  f = &Chip8::OP_NULL;
    for (auto &f : table0) f = &Chip8::OP_NULL;
    for (auto &f : table8) f = &Chip8::OP_NULL;
    for (auto &f : tableE) f = &Chip8::OP_NULL;
    for (auto &f : tableF) f = &Chip8::OP_NULL;

    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1NNN;
    table[0x2] = &Chip8::OP_2NNN;
    table[0x3] = &Chip8::OP_3XNN;
    table[0x4] = &Chip8::OP_4XNN;
    table[0x5] = &Chip8::OP_5XY0;
    table[0x6] = &Chip8::OP_6XNN;
    table[0x7] = &Chip8::OP_7XNN;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9XY0;
    table[0xA] = &Chip8::OP_ANNN;
    table[0xB] = &Chip8::OP_BNNN;
    table[0xC] = &Chip8::OP_CXNN;
    table[0xD] = &Chip8::OP_DXYN;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    // 0x0***
    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    // 0x8***
    table8[0x0] = &Chip8::OP_8XY0;
    table8[0x1] = &Chip8::OP_8XY1;
    table8[0x2] = &Chip8::OP_8XY2;
    table8[0x3] = &Chip8::OP_8XY3;
    table8[0x4] = &Chip8::OP_8XY4;
    table8[0x5] = &Chip8::OP_8XY5;
    table8[0x6] = &Chip8::OP_8XY6;
    table8[0x7] = &Chip8::OP_8XY7;
    table8[0xE] = &Chip8::OP_8XYE;

    // 0xE***
    tableE[0xE] = &Chip8::OP_EX9E;
    tableE[0x1] = &Chip8::OP_EXA1;

    // 0xF***
    tableF[0x07] = &Chip8::OP_FX07;
    tableF[0x0A] = &Chip8::OP_FX0A;
    tableF[0x15] = &Chip8::OP_FX15;
    tableF[0x18] = &Chip8::OP_FX18;
    tableF[0x1E] = &Chip8::OP_FX1E;
    tableF[0x29] = &Chip8::OP_FX29;
    tableF[0x33] = &Chip8::OP_FX33;
    tableF[0x55] = &Chip8::OP_FX55;
    tableF[0x65] = &Chip8::OP_FX65;
}
void Chip8::Cycle()
{
	opcode = (memory[pc] << 8u) | memory[pc + 1];
	pc += 2;
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delay_Timer > 0)
	{
		--delay_Timer;
	}

	// Decrement the sound timer if it's been set
	if (sound_Timer > 0)
	{
		--sound_Timer;
	}
}
void Chip8::LoadFontset()
{
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
}
void Chip8::LoadROM(const char* filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) return;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];
    file.read(buffer, size);

    for (std::streamsize i = 0; i < size; ++i)
        memory[START_ADDRESS + i] = static_cast<uint8_t>(buffer[i]);

    delete[] buffer;
}
void Chip8::OP_00E0() // cls
{
    memset(display, 0, sizeof(display));
}
void Chip8::OP_00EE() // ret
{
    --sp;
    pc = stack[sp];
}
void Chip8::OP_1NNN() // jump NNN
{
    uint16_t address = opcode & 0x0FFFu;
    pc = address;
}
void Chip8::OP_2NNN() // call NNN
{
    uint16_t address = opcode & 0x0FFFu;
    stack[sp++] = pc;
    pc = address;
}
void Chip8::OP_3XNN() // skip next instruction if VX == NN
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    if (gpr[Vx] == byte)
        pc += 2;
}
void Chip8::OP_4XNN() // skip next instruction if VX != NN
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    if (gpr[Vx] != byte)
        pc += 2;
}
void Chip8::OP_5XY0() // skip next instruction if VX == VY
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    if(gpr[Vx] == gpr[Vy])
        pc+=2;
}
void Chip8::OP_6XNN() // set the value of Vx as NN
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = (opcode & 0x00FFu);

    gpr[Vx] = NN;
}
void Chip8::OP_7XNN() // add the value of the NN to the value of Vx
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = (opcode & 0x00FFu);

    gpr[Vx] += NN;   
}
void Chip8::OP_8XY0() // set the value of Vx as the value of Vy
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    gpr[Vx]=gpr[Vy];
}
void Chip8::OP_8XY1() // Bitwise OR
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    gpr[Vx]|=gpr[Vy];
}
void Chip8::OP_8XY2() // Bitwise AND
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    gpr[Vx]&=gpr[Vy];
}
void Chip8::OP_8XY3() // Bitwise XOR
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    gpr[Vx]^=gpr[Vy];
}
void Chip8::OP_8XY4() // Addition with vf = 1 on carry
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = gpr[Vx] + gpr[Vy];

    gpr[Vx]=sum&0x00FFu;
    
    gpr[0xF]=(sum&0b0000'0001'0000'0000u) >> 8u;
}
void Chip8::OP_8XY5() // Subtraction with vf = 0 on borrow
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    gpr[0xF]=(gpr[Vx] > gpr[Vy]) ? 1 : 0;

    gpr[Vx]=gpr[Vx]-gpr[Vy];
}
void Chip8::OP_8XY6() // Shift right Vx, vf = least significant bit before shift
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    gpr[0xF] = (gpr[Vx] & 0b0000'0001u);
    gpr[Vx]>>=1;
}
void Chip8::OP_8XY7() // vx =- vy ; vf = 0 on borrow
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    gpr[0xF]=(gpr[Vy] > gpr[Vx]) ? 1 : 0;

    gpr[Vx]=gpr[Vy]-gpr[Vx];
}
void Chip8::OP_8XYE() // vx <<= vy ; vf = old most significant bit
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    gpr[0xF]=(gpr[Vx] & 0b1000'0000u) >> 7u;

    gpr[Vx] <<=1;
}
void Chip8::OP_9XY0() // skip the instruction if Vx != Vy
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;
    uint8_t Vy=(opcode & 0x00F0u) >> 4u;

    if(gpr[Vx] != gpr[Vy])
        pc += 2;
}
void Chip8::OP_ANNN() // give the index register the value of NNN
{
    uint16_t NNN = (opcode & 0x0FFFu);

    ir=NNN;
}
void Chip8::OP_BNNN() // Jump to the address NNN + V0
{
    uint16_t NNN=(opcode & 0x0FFFu);

    pc=NNN + gpr[0x0];
}
void Chip8::OP_CXNN() // Random number 0-255 AND NN ; Vx := random NN
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = (opcode & 0x00FFu) ;

    gpr[Vx] = randByte(randGen) & NN;
}
void Chip8::OP_DXYN() // sprite Vx Vy N ; Vf = 1 on collision
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t N = opcode & 0x000Fu;

    uint8_t x = gpr[Vx] % 64;
    uint8_t y = gpr[Vy] % 32;

    gpr[0xF]=0;

    for(unsigned int r=0;r<N;r++)
    {
        uint8_t spi_1byte=memory[ir + r];
        for(unsigned int c=0;c<8;c++)
        {
            uint8_t spi_1bit=(spi_1byte) & (0b1000'0000u >> c);

            uint8_t rx = x+c;
            uint8_t ry = y+r;

            if (rx >= 64) 
                rx -= 64;
            if (ry >= 32)
                ry -= 32;
            
            uint32_t* screenPixel = &display[ry * 64 + rx];
            // Sprite pixel is on
			if (spi_1bit)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					gpr[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
        }
    }

}
void Chip8::OP_EX9E() // Is a key not presed ?
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;
    
    if(keypad[gpr[Vx]])
        pc +=2;
}
void Chip8::OP_EXA1() // Is a key presed ?
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;
    
    if(!keypad[gpr[Vx]])
        pc +=2;
}
void Chip8::OP_FX07() // Vx := delay
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;

    gpr[Vx] = delay_Timer;
}
void Chip8::OP_FX0A() // Wait for a keypress
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;

    if (keypad[0])
	{
		gpr[Vx] = 0;
	}
	else if (keypad[1])
	{
		gpr[Vx] = 1;
	}
	else if (keypad[2])
	{
		gpr[Vx] = 2;
	}
	else if (keypad[3])
	{
		gpr[Vx] = 3;
	}
	else if (keypad[4])
	{
		gpr[Vx] = 4;
	}
	else if (keypad[5])
	{
		gpr[Vx] = 5;
	}
	else if (keypad[6])
	{
		gpr[Vx] = 6;
	}
	else if (keypad[7])
	{
		gpr[Vx] = 7;
	}
	else if (keypad[8])
	{
		gpr[Vx] = 8;
	}
	else if (keypad[9])
	{
		gpr[Vx] = 9;
	}
	else if (keypad[10])
	{
		gpr[Vx] = 10;
	}
	else if (keypad[11])
	{
		gpr[Vx] = 11;
	}
	else if (keypad[12])
	{
		gpr[Vx] = 12;
	}
	else if (keypad[13])
	{
		gpr[Vx] = 13;
	}
	else if (keypad[14])
	{
		gpr[Vx] = 14;
	}
	else if (keypad[15])
	{
		gpr[Vx] = 15;
	}
	else
	{
		pc -= 2;
	}
}
void Chip8::OP_FX15() // delay := Vx
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;

    delay_Timer = gpr[Vx];
}
void Chip8::OP_FX18() // buzzer := vx
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;

    sound_Timer = gpr[Vx];
}
void Chip8::OP_FX1E() // i += Vx
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;

    ir+=gpr[Vx];
}
void Chip8::OP_FX29() // Set i to a hex character
{
    uint8_t Vx=(opcode & 0x0F00u) >> 8u;

    ir = FONTSET_START_ADDRESS + (5 * gpr[Vx]);
}
void Chip8::OP_FX33() // Decode vx into binary-coded decimal
{

    int8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = gpr[Vx];

	// Ones-place
	memory[ir + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[ir + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[ir] = value % 10;
}
void Chip8::OP_FX55() // Save V0-VX to i through (i+x) in the memory
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
		memory[ir + i] = gpr[i];
}
void Chip8::OP_FX65() // Read V0-VX to i through (i+x) from the memory
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
		gpr[i] = memory[ir + i];
}


class Platform
{
public:
	Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
	{
		SDL_Init(SDL_INIT_VIDEO);

		window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		texture = SDL_CreateTexture(
			renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
	}

	~Platform()
	{
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void Update(void const* buffer, int pitch)
	{
		SDL_UpdateTexture(texture, nullptr, buffer, pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	}

	bool ProcessInput(uint8_t* keys)
	{
		bool quit = false;

		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				{
					quit = true;
				} break;

				case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						{
							quit = true;
						} break;

						case SDLK_x:
						{
							keys[0] = 1;
						} break;

						case SDLK_1:
						{
							keys[1] = 1;
						} break;

						case SDLK_2:
						{
							keys[2] = 1;
						} break;

						case SDLK_3:
						{
							keys[3] = 1;
						} break;

						case SDLK_q:
						{
							keys[4] = 1;
						} break;

						case SDLK_w:
						{
							keys[5] = 1;
						} break;

						case SDLK_e:
						{
							keys[6] = 1;
						} break;

						case SDLK_a:
						{
							keys[7] = 1;
						} break;

						case SDLK_s:
						{
							keys[8] = 1;
						} break;

						case SDLK_d:
						{
							keys[9] = 1;
						} break;

						case SDLK_z:
						{
							keys[0xA] = 1;
						} break;

						case SDLK_c:
						{
							keys[0xB] = 1;
						} break;

						case SDLK_4:
						{
							keys[0xC] = 1;
						} break;

						case SDLK_r:
						{
							keys[0xD] = 1;
						} break;

						case SDLK_f:
						{
							keys[0xE] = 1;
						} break;

						case SDLK_v:
						{
							keys[0xF] = 1;
						} break;
					}
				} break;

				case SDL_KEYUP:
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_x:
						{
							keys[0] = 0;
						} break;

						case SDLK_1:
						{
							keys[1] = 0;
						} break;

						case SDLK_2:
						{
							keys[2] = 0;
						} break;

						case SDLK_3:
						{
							keys[3] = 0;
						} break;

						case SDLK_q:
						{
							keys[4] = 0;
						} break;

						case SDLK_w:
						{
							keys[5] = 0;
						} break;

						case SDLK_e:
						{
							keys[6] = 0;
						} break;

						case SDLK_a:
						{
							keys[7] = 0;
						} break;

						case SDLK_s:
						{
							keys[8] = 0;
						} break;

						case SDLK_d:
						{
							keys[9] = 0;
						} break;

						case SDLK_z:
						{
							keys[0xA] = 0;
						} break;

						case SDLK_c:
						{
							keys[0xB] = 0;
						} break;

						case SDLK_4:
						{
							keys[0xC] = 0;
						} break;

						case SDLK_r:
						{
							keys[0xD] = 0;
						} break;

						case SDLK_f:
						{
							keys[0xE] = 0;
						} break;

						case SDLK_v:
						{
							keys[0xF] = 0;
						} break;
					}
				} break;
			}
		}

		return quit;
	}

private:
	SDL_Window* window{};
	SDL_Renderer* renderer{};
	SDL_Texture* texture{};
};

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	char const* romFilename = argv[3];

	Platform platform("CHIP-8 Emulator", 64 * videoScale, 32 * videoScale, 64, 32);

	Chip8 chip8;
	chip8.LoadROM(romFilename);

	int videoPitch = sizeof(chip8.display[0]) * 64;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;

	while (!quit)
	{
		quit = platform.ProcessInput(chip8.keypad);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay)
		{
			lastCycleTime = currentTime;

			chip8.Cycle();

			platform.Update(chip8.display, videoPitch);
		}
	}

	return 0;
}