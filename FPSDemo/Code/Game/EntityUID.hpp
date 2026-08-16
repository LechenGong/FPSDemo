#pragma once

struct EntityUID
{
public:
	EntityUID();
	EntityUID( unsigned int salt, unsigned int index );

	bool isValid() const;
	unsigned int GetIndex() const;
	unsigned int GetSalt() const;
	bool operator==( EntityUID const& other ) const;
	bool operator!=( EntityUID const& other ) const;

	static const EntityUID INVALID;

private:
	unsigned int m_data = INVALID.m_data;
};
