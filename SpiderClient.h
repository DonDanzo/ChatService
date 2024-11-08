#pragma once
#include "Client.h"

class SpiderClient : public Client
{
	public:
		void ProcessIncomeMessages() override;
};

