#pragma once

#include <vector>
#include <memory>

#include "actor.h"

class troupe : public actor
{
public:
    troupe();
    virtual ~troupe();
    virtual void draw(effect& fx, D3DXMATRIX mat) const;
    void insert(const std::shared_ptr<actor>&);
protected:
    troupe(const troupe&);
    troupe& operator=(const troupe&);
private:
    std::vector<std::shared_ptr<actor> > actors;
};
