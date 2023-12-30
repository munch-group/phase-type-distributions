#ifndef CCONSTRAINEDNJLIB_H
#define CCONSTRAINEDNJLIB_H



// Here, we overload the << operator so it works on any set (python style)
template <typename TYPE>
inline std::ostream &operator<<(std::ostream &o, std::set<TYPE> s) {
     o << "set([";
     for (typename std::set<TYPE>::iterator it = s.begin(); it != s.end(); ++it) {
       if (it != s.begin() && it != s.end())
	 o << ", ";	  
       o<<*it;
     }
     o << "])";     
     return o;
}

template<class T, class Comp>
bool set_member(T x, std::set<T,Comp>& s)
// Test if int in in set of ints.
{
 return (s.count(x)==1 ? true : false);
}

// std::set<int> set_intersection(std::set<int>, std::set<int>);
std::set<int> set_union(std::set<int>, std::set<int>);
bool set_includes(std::set<int>, std::set<int>);
bool pair_identity (int *, int *);
bool pair_sort (int *, int *);
void printPairsSet(std::set<int *>);
int intMax(int, int);
int intMin(int, int );
void swapInts(int *, int *);
void printDistMatrix(void);
void printQmatrix(void);
bool findInIntVector(int, std::vector<int>);
std::vector<int*> get_pairwise_combinations(std::vector<int>);
std::vector<std::string> stringSplit(std::string str, std::string);







/* template<typename T>
 * inline bool isanyinf(T value)
 * {
 *   return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
 * }
 */

class TooFewAlnBasesError: public std::exception
{
  virtual const char* what() const throw()
  {
    return "TooFewAlnBasesError";
  }
} TooFewAlnBasesError; // This name can also be a short name for the class. It has to bere there for some reason so we just make it hte same as the class name.

class Node {
public:
  Node();
/*   ~Node();
 */
  Node *left;
  Node *right;     /* right children, or None */
  void assignLeft (Node);
  void assignRight (Node);
  Node getRight (void);
  Node getLeft (void);
  int name;
  double distLeft;  /* Length of edges to the children, if any */
  double distRight;
  std::set<int> leafSet;     /* list of subnodes as set in each node */
  char *getSubTreeString(char *);
  int branchLengths; /* should we return calculated branchlengths? 0/1 */
};  

class K2Pstats {
public:
  int transitions;
  int transversions;
  int alignedBases;
};

K2Pstats k2pStats(int, int, int, char **, int *);
double k2pDist(K2Pstats, int, char **, int *);
double computeDistance(int, int, int, char **, int *);
double resampleDistance(int, int, int, char **, int *);
double calcRsum(int, double **);
void computeAllowedPairs(void);
void updateAllowedPairs(int, int);
bool updateConstraints(int, int);
void updateMatrices(int, int);
int *findPair(double **, std::list<int *>);
void createParentNode(int, int);
extern "C" char *compute(int, char **, int, char **, int, int);
//extern "C" void initCache(int, double **);
extern "C" void initCache(int);
extern "C" void deleteCache(int);

int main(void);

#endif
