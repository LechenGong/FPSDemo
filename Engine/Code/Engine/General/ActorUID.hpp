#pragma once

struct ActorUID
{
public:
	ActorUID();
	ActorUID( unsigned int salt, unsigned int index );

	bool isValid() const;
	unsigned int GetIndex() const;
	unsigned int GetSalt() const;
	bool operator==( ActorUID const& other ) const;
	bool operator!=( ActorUID const& other ) const;

	static const ActorUID INVALID;

private:
	unsigned int m_data = INVALID.m_data;
};
