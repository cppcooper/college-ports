#ifndef _ENEMIES_H_
#define _ENEMIES_H_

#include <space_invaders/objects/screen_objs.h>

class Archie : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Archie();
	virtual ~Archie();
	void deleteSelf() final override { delete this; }
};

class Mal : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Mal();
	virtual ~Mal();
	void deleteSelf() final override { delete this; }
};

class Kriege : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Kriege();
	virtual ~Kriege();
	void deleteSelf() final override { delete this; }
};

class Lana : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Lana();
	virtual ~Lana();
	void deleteSelf() final override { delete this; }
};

class Cheryl : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Cheryl();
	virtual ~Cheryl();
	void deleteSelf() final override { delete this; }
};

class Cyril : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Cyril();
	virtual ~Cyril();
	void deleteSelf() final override { delete this; }
};

class Ray : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Ray();
	virtual ~Ray();
	void deleteSelf() final override { delete this; }
};

class Woodie : public Enemy
{
private:
	static unsigned int count;
protected:
public:
	Woodie();
	virtual ~Woodie();
	void deleteSelf() final override { delete this; }
};

#endif
