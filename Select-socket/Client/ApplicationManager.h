#pragma once

class ApplicationManager final
{
public:
	ApplicationManager();
	ApplicationManager(ApplicationManager& other) = delete;
	void operator=(ApplicationManager& other) = delete;
	~ApplicationManager();

	void Update(Message* outMessage);
	void ProcessMessage(const Message& message);
	const std::vector<std::unique_ptr<Player>>& GetPlayerList() const;
private:
	Player* findPlayerOrNull(int32_t targetID) const;

private:
	int32_t mUserID;
	std::vector<std::unique_ptr<Player>> mPlayerList;
};