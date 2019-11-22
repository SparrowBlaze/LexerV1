//
//  Relation.cpp
//  LexerV1
//
//  Created by James Robinson on 11/4/19.
//  Copyright © 2019 James Robinson. All rights reserved.
//

#include "Relation.h"

Relation::Relation(const Relation &other) {
    this->name = other.name;
    this->contents = other.contents;
    this->scheme = other.scheme;
}

Relation::Relation(const std::string name, Tuple scheme) {
    this->name = name;
    this->contents = std::set<Tuple>();
    this->scheme = scheme;
}

Relation::~Relation() {
    contents.clear();
}

std::string Relation::getName() const {
    return this->name;
}

void Relation::setName(const std::string newName) {
    this->name = newName;
}

size_t Relation::getColumnCount() const {
    return getScheme().size();
}

Tuple Relation::getScheme() const {
    return scheme;
}

bool Relation::addTuple(Tuple element) {
    if (element.size() != getColumnCount()) {
        return false;
    }
    
    this->contents.insert(element);
    return true;
}

std::set<Tuple> Relation::getContents() const {
    return this->contents;
}

std::vector<Tuple> Relation::listContents() const {
    std::vector<Tuple> result = {};
    
    for (auto t : getContents()) {
        result.push_back(t);
    }
    
    return result;
}

Relation Relation::rename(std::string oldCol, std::string newCol) const {
    Tuple newScheme = getScheme();
    
    for (size_t i = 0; i < newScheme.size(); i += 1) {
        if (newScheme.at(i) == oldCol) {
            newScheme.at(i) = newCol;
        }
    }
    
    return rename(newScheme);
}

Relation Relation::rename(Tuple newScheme) const {
    if (newScheme.size() != getScheme().size() || // Wrong size, or
        newScheme == getScheme()) { // Identical scheme
        return *this;
    }
    
    Tuple resultScheme = getScheme();
    std::set<std::string> resultValues = std::set<std::string>();
    
    for (size_t col = 0; col < newScheme.size(); col += 1) {
        std::string oldVal = getScheme().at(col);
        std::string newVal = newScheme.at(col);
        
        if (!newVal.empty()) {
            resultScheme.at(col) = newVal;
            resultValues.insert(newVal);
        } else {
            resultValues.insert(oldVal);
        }
    }
    
    if (resultValues.size() == resultScheme.size()) {
        // We have no duplicates! Send it off.
        Relation result = Relation(name, resultScheme);
        result.contents = getContents();
        return result;
        
    } else {
        // Duplicate values! Do nothing.
        return *this;
    }
}

Relation Relation::select(std::vector< std::pair<size_t, std::string> > queries) const {
    Relation result = Relation(getName(), getScheme());
    
    // Evaluate each tuple
    for (auto t : getContents()) {
        bool isMatch = true;

        for (auto query : queries) {
            size_t col = query.first;
            std::string val = query.second;
            
            if (col >= result.getColumnCount()) {
                continue; // Too big? Next query.
            }
            if (t.at(col) != val) {
                isMatch = false;
                break; // Column doesn't match an expected value? Next tuple.
            }
        }
        
        if (isMatch) {
            result.addTuple(t);
        }
    }
    
    return result;
}

Relation Relation::select(std::vector< std::vector<size_t> > queries) const {
    Relation result = Relation(getName(), getScheme());
    
    // Evaluate each equivalence
    for (std::vector<size_t> query : queries) {
        
        if (query.empty()) { // No query? Select everything.
            result.contents = getContents();
            break;
        }
        
        // Make sure that each of the named columns carry the same value, if they're in range.
        for (auto t : getContents()) {
            bool hasMatch = true;
            std::string val = "LexerV1.INVALID";
            
            for (auto col : query) {
                if (col >= result.getColumnCount()) {
                    continue; // Too big? Move along.
                }
                
                if (val == "LexerV1.INVALID") {
                    val = t.at(col);
                }
                
                // If we don't have a match, skip along.
                if (t.at(col) != val) {
                    hasMatch = false;
                    break;
                }
            }
            
            if (hasMatch) {
                result.addTuple(t);
            }
        }
    }
    
    return result;
}

bool Relation::vectorContainsValue(const std::vector<std::string> &domain,
                                   const std::string &query) const {
    for (auto val : domain) {
        if (val == query) {
            return true;
        }
    }
    
    return false;
}

void Relation::stripExtraColsFromScheme(Tuple &otherScheme) const {
    // Evaluate each column in given scheme.
    Tuple result = {};
    
    if (otherScheme.empty()) {
        return; // Empty scheme? That's ok.
    }
    
    Tuple dirtyScheme = otherScheme;
    otherScheme.clear();
    
    // Strip duplicates from otherScheme
    for (auto col : dirtyScheme) {
        if (!vectorContainsValue(otherScheme, col)) {
            otherScheme.push_back(col);
        }
    }
    
    // Pull only those from otherScheme which are in current scheme
    for (auto col : otherScheme) {
        if (vectorContainsValue(getScheme(), col)) {
            result.push_back(col);
        }
    }
    
    otherScheme = result;
}

int Relation::indexForColumnInScheme(std::string col) const {
    return indexForColumnInTuple(col, getScheme());
}

int Relation::indexForColumnInTuple(std::string col, const Tuple &domain) const {
    for (unsigned int i = 0; i < domain.size(); i += 1) {
        if (domain.at(i) == col) {
            return i;
        }
    }
    
    return -1;
}

void Relation::swapColumns(size_t oldCol, size_t newCol) {
    if (oldCol >= getColumnCount()) {
        oldCol = getColumnCount() - 1; // Too large? Use the end instead.
    }
    
    if (newCol >= getColumnCount()) {
        newCol = getColumnCount() - 1; // Too large? Use the end instead.
    }
    
    if (oldCol == newCol) {
        return; // Swap with self? Done.
    }
    
    // Reorder scheme
    std::string val = scheme.at(oldCol);
    scheme.at(oldCol) = scheme.at(newCol);
    scheme.at(newCol) = val;
    
    // Reorder each tuple
    std::set<Tuple> reordered = std::set<Tuple>();
    for (auto t : contents) {
        std::string val = t.at(oldCol);
        t.at(oldCol) = t.at(newCol);
        t.at(newCol) = val;
        reordered.insert(t);
    }
    contents = reordered;
}

void Relation::keepOnlyColumnsUntil(size_t col) {
    if (col >= getColumnCount()) {
        return;
    }
    
    // Strip from scheme
    scheme.erase(scheme.begin() + col, scheme.end());
    
    std::set<Tuple> stripped = std::set<Tuple>();
    for (auto t : contents) {
        
        // Strip from each column
        t.erase(t.begin() + col, t.end());
        stripped.insert(t);
        
    }
    
    // If we only have a single empty row, remove it
    if (stripped.size() == 1 && (*stripped.begin()).empty()) {
        stripped.clear();
    }
    
    contents = stripped;
}

Relation Relation::project(Tuple scheme) const {
    Tuple current = getScheme();
    Tuple newScheme = scheme;
    
    stripExtraColsFromScheme(newScheme); // This removes columns that don't exist.
    
    Relation result = Relation(*this);
    
    // For each column in scheme, find where it was in our old scheme, then apply.
    for (unsigned int newIndex = 0; newIndex < newScheme.size(); newIndex += 1) {
        size_t oldIndex = result.indexForColumnInScheme(newScheme.at(newIndex));
        result.swapColumns(oldIndex, newIndex);
    }
    
    result.keepOnlyColumnsUntil(newScheme.size());
    
    return result;
}

std::string Relation::stringForTuple(Tuple tuple) const {
    if (tuple.size() != getColumnCount()) {
        return "";
    }
    
    std::ostringstream result = std::ostringstream();
    for (unsigned int i = 0; i < tuple.size(); i += 1) {
        std::string col = getScheme().at(i);
        std::string val = tuple.at(i);
        
        result << col << "=" << val;
        if (i < getScheme().size() - 1) {
            // If more columns, add a comma
            result << ", ";
        }
    }
    
    return result.str();
}

Relation Relation::joinedWith(Relation other) const {
    if (this->getName() == other.getName() &&
        this->getScheme() == other.getScheme() &&
        this->getContents() == other.getContents()) {
        return *this;
    }
    
    Tuple newScheme = getScheme().combinedWith(other.getScheme());
    Relation result = Relation(getName(), newScheme);
    
    for (Tuple t1 : this->getContents()) {
        for (Tuple t2 : other.getContents()) {
            
            Tuple combined = newScheme;
            bool isValid = true;
            for (size_t colIdx = 0; colIdx < combined.size(); colIdx += 1) {
                // For each column,
                std::string col = combined.at(colIdx);
                
                //   Find that item in each relation
                int index1 = this->indexForColumnInScheme(col);
                std::string val1 = "";
                int index2 = other.indexForColumnInScheme(col);
                std::string val2 = "";
                
                if (index1 >= 0) {
                    val1 = t1.at(index1);
                }
                if (index2 >= 0) {
                    val2 = t2.at(index2);
                }
                
                if ((val1 == val2 && !val1.empty()) || // If val is identical, or
                    (!val1.empty() && val2.empty())) { // If val is found only in t1,
                    // Add t1's value to our combined tuple.
                    combined.at(colIdx) = val1;
                }
                if ((val1 == val2 && !val2.empty()) || // If val is identical, or
                    (!val2.empty() && val1.empty())) { // If val is found only in t2,
                    // Add t2's value to our combined tuple.
                    combined.at(colIdx) = val2;
                }
                if (val1 != val2 && !val1.empty() && !val2.empty()) { // If they're different,
                    // Drop the tuple
                    isValid = false;
                }
            }
            
            if (isValid) {
                result.addTuple(combined);
            }
            
        }
    }
    
    return result;
}

Relation Relation::unionWith(Relation other) const {
    if (other.getScheme() != getScheme()) {
        // If we aren't union-compatible, return an empty table.
        return Relation(getName(), getScheme());
    }
    
    Relation result = Relation(getName(), getScheme());
    
    for (auto t : getContents()) {
        result.addTuple(t);
    }
    for (auto t : other.getContents()) {
        result.addTuple(t);
    }
    
    return result;
}

bool Relation::operator ==(const Relation &other) {
    return (getName() == other.getName() &&
            getScheme() == other.getScheme() &&
            getContents() == other.getContents());
}

bool Relation::operator !=(const Relation &other) {
    return !(*this == other);
}
