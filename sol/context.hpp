#pragma once

namespace sol 
{

	class Context 
	{
	public:
		Context();
		void ~Context();

	private:
		static uint32_t s_counter = 0;

	};

}