/*
   *) fold -1 constants into the expression:

   + ( C_\phi S_\phi + C_\phi S_\phi ) (  -  1 )
   + ( C_\phi S_\phi - C_\phi S_\phi ) (  -  1 \mathbf{e}_1 \wedge \mathbf{e}_3 )

   *) NICE TO HAVE (may have to modify e3ga though): format the following as just the expression (ie: when the mv value == 1)

    ( C_\phi^2 - S_\phi^2 ) (  1 )

   *) BUG:

from R_33. This should have been reduced to zero:

+ 2 (C_\phi C_\psi S_\phi S_\psi S_\theta^2 )
 -2 (C_\phi C_\psi S_\phi S_\psi S_\theta^2 )
+ 2 (C_\phi C_\psi C_\theta^2 S_\phi S_\psi )
 -2 (C_\phi C_\psi C_\theta^2 S_\phi S_\psi )

 */
#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <map>

#include <libgasandbox/common.h>
#include <libgasandbox/e3ga.h>

using namespace e3ga ;
using namespace std ;

#if 0
float foopeekasm( const mv & v )
{
//
// interesting.  looking at the generated asm shows that the compiler
// (g++ -O2) is
// able to inline this function despite the fact that the body of
// mv::gu() is not available until after it is used.
//
// Compilers are getting better!
//
  return v.e3e1() ;
}
#endif

typedef std::string literal ;

/**
   \brief A class representing one of the elements in a sum.

   logically this has the form:

   scalar [literal^exponent]*

   This is a scaling term (default of one).  This is followed by zero
   or more symbolic literals raised to integer powers (default of one).
 */
class term
{
   typedef map<literal, int> factorsType ;
   typedef factorsType::iterator iterType ;
   typedef factorsType::const_iterator citerType ;

   #define TERM_ZERO_VALUE 0
   #define TERM_ONE_VALUE 1

public:
   // TODO: perhaps make this float?  int is good enough for what I want right now.
   typedef int scaleType ;

private:
   factorsType m_factors ;
   scaleType m_scalar ;

public:
   /**
    * construct a scalar.
    */
   term( const scaleType & v ) : m_scalar(v)
   {
   }

   /**
    * construct term from a literal.
    */
   term( const literal & literal ) : m_scalar(TERM_ONE_VALUE)
   {
      m_factors[ literal ] = 1 ;
   }

   /**
    * reset the scale factor to one.
    */
   void normalize()
   {
      m_scalar = TERM_ONE_VALUE ;
   }

#if 0
   void adjustScale( const scaleType s )
   {
      m_scalar += s ;
   }
#endif

   scaleType getScale() const
   {
      return m_scalar ;
   }

   /**
    * add to the scalar value for this set of factors using the scale factor from another (presumed equal term).
    */
   void adjustScale( const term & t )
   {
      m_scalar += t.m_scalar ;
   }

   /**
    * multiply term by a single literal.  If this shared any common factors with this term, that factors' exponent will be incremented.
    */
   term & operator *= ( const literal & v )
   {
      m_factors[ v ] ++ ;

      return *this ;
   }

   /**
    * multiply term by another term.  Any common factors will result in aggreggate exponentation.
    */
   term & operator *= ( const term & v )
   {
      for ( citerType i = v.m_factors.begin() ; i != v.m_factors.end() ; i++ )
      {
         m_factors[ i->first ] += i->second ;
      }
      m_scalar *= v.m_scalar ;

      return *this ;
   }

   /**
    * adjust the sign of the scalar factor for this term.
    */
   void negate()
   {
      m_scalar *= -TERM_ONE_VALUE ;
   }

   /**
    * convert term to a string.
    */
   inline std::string toString(void) const ;

   void toStringStream( std::ostringstream & out, const bool useSignPrefix ) const ;

   /**
    * compare two terms for sort purposes.
    */
   friend bool compareTerm( const term & a, const term & b ) ;

   bool isZero() const
   {
      return ( TERM_ZERO_VALUE == m_scalar ) ;
   }
} ;

void term::toStringStream( std::ostringstream & out, const bool useSignPrefix ) const
{
   if ( m_factors.size() )
   {
      if ( useSignPrefix && m_scalar > TERM_ZERO_VALUE )
      {
         out << " + " ;
      }

      if ( -TERM_ONE_VALUE == m_scalar )
      {
         out << " - " ;
      }
      else if ( m_scalar != TERM_ONE_VALUE )
      {
         out << m_scalar << " (" ;
      }

      bool doneFirst = false ;

      for ( citerType i = m_factors.begin() ; i != m_factors.end() ; i++ )
      {
         if ( doneFirst )
         {
            out << " " ;
         }
         else
         {
            doneFirst = true ;
         }

         if ( 1 == i->second )
         {
            out << i->first ;
         }
         else
         {
            out << i->first << "^" << i->second ;
         }
      }

      if ( m_scalar != TERM_ONE_VALUE && m_scalar != -TERM_ONE_VALUE )
      {
         out << " )" ;
      }
   }
   else
   {
      if ( useSignPrefix && m_scalar > TERM_ZERO_VALUE )
      {
         out << " + " ;
      }

      out << m_scalar ;
   }
}

inline std::string term::toString(void) const
{
   std::ostringstream out ;

   toStringStream( out, false ) ;

   return out.str() ;
}

inline bool compareTerm( const term & a, const term & b )
{
   std::string aStr = a.toString() ;
   std::string bStr = b.toString() ;

   return aStr > bStr ;
}

/**
 * A representation of a set (sum) of terms.
 *
 * Addition and subtraction are implemented.
 * Multiplication (with distribution over all sums) is implemented.
 *
 * Addition and subtraction operations do not result in common expression elimination, and reduce() must be called
 * explicitly if desired.
 */
class expression
{
   typedef std::list<term> contType ;
   typedef contType::iterator iterType ;
   typedef contType::const_iterator citerType ;

   contType m_summands ;

   /**
    * multiply self by complete expression.  No
    */
   void multiply( expression & r, const expression & a, const expression & b )
   {
      // TODO: assert that r.m_summands.size() == 0

      // c = \sum_i a_i b_i
      for ( citerType i = a.m_summands.begin() ; i != a.m_summands.end() ; i++ )
      {
         for ( citerType j = b.m_summands.begin() ; j != b.m_summands.end() ; j++ )
         {
            term tmp(*i) ;
//      cout << "e:mult: *i: " << (*i).toString() << endl ;
//      cout << "e:mult: *j: " << (*j).toString() << endl ;

            tmp *= (*j) ;
//      cout << "post: e:mult: tmp: " << tmp.toString() << endl ;

            r.m_summands.push_front(tmp) ;
         }
      }
   }

   // for operator *='s use of swap.  Likely no other uses for empty expressions.
   expression( )
   {
   }

public:

   expression( const term & t )
   {
      m_summands.push_front( t ) ;
   }

   /**
    * multiply all terms in self by factor.
    */
   expression & operator *= ( const term & factor )
   {
      for ( iterType i = m_summands.begin() ; i != m_summands.end() ; i++ )
      {
         (*i) *= factor ;
      }

      return *this ;
   }

   std::string toString() const ;

   expression & operator *= ( const expression & e )
   {
      expression t ;
      t.m_summands.swap( m_summands ) ;

//      cout << "e *= t: " << t.toString() << endl ;
//      cout << "e *= *this: " << toString() << endl ;
//      cout << "e *= e: " << e.toString() << endl ;

      multiply( *this, t, e ) ;

//      cout << "post: e *= *this: " << toString() << endl ;

      return *this ;
   }

   /**
    * negate all terms in self.
    */
   void negate()
   {
      for ( iterType i = m_summands.begin() ; i != m_summands.end() ; i++ )
      {
         (*i).negate() ;
      }
   }

   /**
    * add a term from self.
    */
   expression & operator += ( const term & t )
   {
      m_summands.push_front( t ) ;

      return *this ;
   }

   /**
    * subtract a term from self.
    */
   expression & operator -= ( const term & t )
   {
      term n(t) ;
      n.negate() ;

      m_summands.push_front( n ) ;

      return *this ;
   }

   /**
    * add an expression from self.
    */
   expression & operator += ( const expression & e )
   {
      for ( citerType i = e.m_summands.begin() ; i != e.m_summands.end() ; i++ )
      {
         m_summands.push_front((*i)) ;
      }

      return *this ;
   }

   /**
    * subtract an expression from self.
    */
   expression & operator -= ( const expression & e )
   {
      for ( citerType i = e.m_summands.begin() ; i != e.m_summands.end() ; i++ )
      {
         term n((*i)) ;
         n.negate() ;
         m_summands.push_front(n) ;
      }

      return *this ;
   }

   /**
    * eliminate common expressions.
    */
   void reduce() ;

   /**
    * is an expression equal to zero.  Assumes that the expression is reduced.
    */
   bool isZero() const ;
} ;

bool expression::isZero() const
{
   citerType i = m_summands.begin() ;

   if ( (*i).isZero() )
   {
      i++ ;

      if ( i == m_summands.end() )
      {
         return true ;
      }
   }

   return false ;
}

void expression::reduce()
{
   m_summands.sort( compareTerm ) ;

   std::string curStr ;
   iterType prev ;
   iterType i = m_summands.begin() ;

   while ( i != m_summands.end() && (TERM_ZERO_VALUE == (*i).getScale()) )
   {
      i = m_summands.erase( i ) ;
   }

   if ( i != m_summands.end() )
   {

      {
         term curValue = *i ;
         curValue.normalize() ;
         curStr = curValue.toString() ;
      }

      prev = i ;
      i++ ;
      while ( i != m_summands.end() )
      {
         std::string nextStr ;

         while ( i != m_summands.end() && (TERM_ZERO_VALUE == (*i).getScale()) )
         {
            i = m_summands.erase( i ) ;
         }

         if ( i == m_summands.end() )
         {
            break ;
         }

         {
            term next = *i ;
            next.normalize() ;
            nextStr = next.toString() ;
         }

         if ( nextStr == curStr )
         {
            (*prev).adjustScale( *i ) ;

            i = m_summands.erase( i ) ;
         }
         else
         {
            prev = i ;
            curStr = nextStr ;
            i++ ;
         }
      }
   }

   if ( !m_summands.size() )
   {
      m_summands.push_front( term( TERM_ZERO_VALUE ) ) ;
   }
}

std::string expression::toString() const
{
   std::ostringstream out ;

   bool wantSignPrefix = false ;

   contType::size_type size = m_summands.size() ;

   if ( size > 1 )
   {
      out << "( " ;
   }

   for ( citerType i = m_summands.begin() ; i != m_summands.end() ; i++ )
   {
      (*i).toStringStream( out, wantSignPrefix ) ;

      wantSignPrefix = true ;
   }

   if ( size > 1 )
   {
      out << " )" ;
   }

   return out.str() ;
}

class symbol ;
bool compareSymbol (const symbol & first, const symbol & second) ;

class sum ;
sum dot( const sum & a, const mv & b ) ;

class symbol
{
   expression m_symName ;
   typedef mv valueType ;
   valueType m_symNumeric ;

   friend class sum ;

public:
   symbol( const expression & name, const valueType & value ) : m_symName(name), m_symNumeric(value)
   {
   }

   symbol( const term & name, const valueType & value ) : m_symName(name), m_symNumeric(value)
   {
   }

   symbol( const literal & name, const valueType & value ) : m_symName(term(name)), m_symNumeric(value)
   {
   }

   symbol( const valueType & value ) : m_symName(term(1)), m_symNumeric(value)
   {
   }

   symbol( const literal & name ) : m_symName(term(name)), m_symNumeric(1)
   {
   }

   symbol( const term::scaleType & scalar ) : m_symName(term(scalar)), m_symNumeric(1)
   {
   }

   symbol & operator *= ( const valueType & scale )
   {
      m_symNumeric *= scale ;

      return *this ;
   }

#if 0
   symbol & operator *= ( const expression & scale )
   {
      m_symName *= scale ;

      return *this ;
   }
#endif

   symbol & operator *= ( const symbol & scale )
   {
      m_symName *= scale.m_symName ;
      m_symNumeric *= scale.m_symNumeric ;

      return *this ;
   }

   void reduce()
   {
      m_symName.reduce() ;
   }

   void reverseMe()
   {
      m_symNumeric = reverse( m_symNumeric ) ;
   }

   friend bool compareSymbol (const symbol & first, const symbol & second) ;
   friend sum dot( const sum & a, const mv & b ) ;

   void dump() const ;

   #define SYMBOL_SCALAR_PART    (1 << 0)
   #define SYMBOL_VECTOR_PART    (1 << 1)
   #define SYMBOL_BIVECTOR_PART  (1 << 2)
   #define SYMBOL_TRIVECTOR_PART (1 << 3)
   /**
      \param mask [in]
         One of SYMBOL_SCALAR_PART, SYMBOL_VECTOR_PART, SYMBOL_BIVECTOR_PART, or SYMBOL_TRIVECTOR_PART.
    */
   void gradeFilter( const unsigned int mask )
   {
      m_symNumeric.m_gu &= mask ;
   }

   /**
    * determine if a reduced symbol is zero (either a zero scalar value in the the one and only term of the expression, 
    * or a zero as the coefficient of the mv value).
    */
   bool isZero() const ;
} ;

/**
 * return a bitmask with a bit set for each unique basis element in the algebra.
 * Look in the mv internals ... this operation very likely already exists.
 */
int mv_bitmask( const mv & a )
{
   int m = 0 ;
   int k = 0 ;
   int b = 0 ;
   for ( int i = 0 ; i <= 3 ; i++ )
   {
      if ( a.gu() & (1 << i) )
      {
         for ( int j = 0 ; j < mv_gradeSize[i] ; j++ )
         {
            if ( a.m_c[k] )
            {
               m |= (1 << b) ;
            }

            k++ ;
            b++ ;
         }
      }
      else
      {
         b += mv_gradeSize[i] ;
      }
   }

   return m ;
}

bool symbol::isZero() const
{
   if ( m_symName.isZero() || ( 0 == mv_bitmask( m_symNumeric ) ) )
   {
      return true ;
   }
   else
   {
      return false ;
   }
}

void symbol::dump() const
{
   cout << m_symName.toString()
        << " ( "
        << toString(m_symNumeric)
        << " )"
        << endl ;
}


bool compareSymbol (const symbol & first, const symbol & second)
{
   const mv & a = first.m_symNumeric ;
   const mv & b = second.m_symNumeric ;

   if ( a.gu() > b.gu() )
   {
      return false ;
   }
   else if ( a.gu() < b.gu() )
   {
      return true ;
   }
   else
   {
      int la = mv_bitmask( a )  ;
      int lb = mv_bitmask( b )  ;

      return la > lb ;
   }
}

class sum
{
   typedef std::list<symbol> symbolSetType ;
   typedef symbolSetType::iterator iterType ;
   typedef symbolSetType::const_iterator citerType ;

   symbolSetType m_listOfSymbols ;

public:
   sum( ) {}

   sum( const symbol & i )
   {
      m_listOfSymbols.push_front( i ) ;
   }

   sum & operator += ( const symbol & more )
   {
      m_listOfSymbols.push_front( more ) ;

      return *this ;
   }

   sum & operator *= ( const symbol & scale )
   {
      for ( iterType i = m_listOfSymbols.begin() ; i != m_listOfSymbols.end() ; i++ )
      {
         (*i) *= scale ;
      }
   }

#if 0 // not used.
   void reverseMe()
   {
      iterType i = m_listOfSymbols.begin() ;

      while ( i != m_listOfSymbols.end() )
      {
         (*i).reverseMe() ;

         i++ ;
      }
   }
#endif

   sum reverse() const
   {
      sum r(*this) ;
      
      for ( iterType i = r.m_listOfSymbols.begin() ; i != r.m_listOfSymbols.end() ; i++ )
      {
         (*i).reverseMe() ;
      }

      return r ;
   }

   friend sum operator * ( const sum & l, const sum & r )
   {
      sum agg ;
      
      for ( citerType i = l.m_listOfSymbols.begin() ; i != l.m_listOfSymbols.end() ; i++ )
      {
         for ( citerType j = r.m_listOfSymbols.begin() ; j != r.m_listOfSymbols.end() ; j++ )
         {
            symbol tmp(*i) ;
            const symbol & b = (*j) ;
            tmp *= b ;
            agg += tmp ;
         }
      }

      return agg ;
   }

   void reduce( const bool doPostSort = true ) ;

   void dump(void) const ;

   friend sum dot( const sum & a, const mv & b ) ;

   void gradeFilter( const unsigned int mask )
   {
      for ( iterType i = m_listOfSymbols.begin() ; i != m_listOfSymbols.end() ; i++ )
      {
         (*i).gradeFilter( mask ) ;
      }
   }
} ;

void sum::reduce( const bool doPostSort )
{
   m_listOfSymbols.sort( compareSymbol ) ;

   if ( doPostSort )
   {
      iterType i = m_listOfSymbols.begin() ;

      while ( i != m_listOfSymbols.end() )
      {
         iterType j = i ;
         symbol & cur = (*i) ;

         j++ ;

         while ( j != m_listOfSymbols.end() )
         {
            const symbol & next = (*j) ;

            if ( cur.m_symNumeric.gu() && (cur.m_symNumeric.gu() == next.m_symNumeric.gu()) )
            {
               bool match = true ;
               bool nmatch = true ;
               int k = 0 ;
               int ia = 0 ;

               for ( int ii = 0 ; ii <= 3 ; ii++ )
               {
                  if ( cur.m_symNumeric.gu() & (1 << ii) )
                  {
                     for ( int jj = 0 ; jj < mv_gradeSize[ii] ; jj++)
                     {
                        if ( cur.m_symNumeric.m_c[k] != next.m_symNumeric.m_c[k] )
                        {
                           match = false ;
                        }

                        if ( cur.m_symNumeric.m_c[k] != -next.m_symNumeric.m_c[k] )
                        {
                           nmatch = false ;
                        }

                        k++ ;
                        ia++ ;
                     }
                  }
                  else
                  {
                     ia += mv_gradeSize[ii] ;
                  }
               }

               if ( match )
               {
                  cur.m_symName += next.m_symName ;

                  j = m_listOfSymbols.erase(j) ;
               }
               else if ( nmatch )
               {
                  cur.m_symName -= next.m_symName ;

                  j = m_listOfSymbols.erase(j) ;
               }
               else
               {
                  break ;
               }
            }
            else
            {
               break ;
            }
         }

         i = j ;
      }

      // Final pass, now that all the common multivector factors have been accumlated, reduce the symbols.
      for ( i = m_listOfSymbols.begin() ; i != m_listOfSymbols.end() ; )
      {
         (*i).reduce() ;

         if ( (*i).isZero() )
         {
            i = m_listOfSymbols.erase( i ) ;
         }
         else
         {
            i++ ;
         }
      }
   }
}

void sum::dump(void) const
{
   bool first = true ;
   bool zero = true ;

   for ( citerType i = m_listOfSymbols.begin() ; i != m_listOfSymbols.end() ; i++ )
   {
      if ( first )
      {
         cout << "\\left( " << endl ;
      }
      else
      {
         cout << "+ " ;
      }

      zero = false ;
      cout << (*i).m_symName.toString()
           << " ( "
           << toString((*i).m_symNumeric)
           << " )"
           << endl ;

      first = false ;
   }

   if ( zero )
   {
      cout << "0" << endl ;
   }
   else
   {
      cout << "\\right)" << endl ;
   }
}

sum dot( const sum & a, const mv & b )
{
   sum r ;

   for ( sum::citerType i = a.m_listOfSymbols.begin() ; i != a.m_listOfSymbols.end() ; i++ )
   {
      symbol t(*i) ;

      t.m_symNumeric = b << t.m_symNumeric ;

      r.m_listOfSymbols.push_front( t ) ;
   }

   return r ;
}

int main(int argc, char*argv[])
{
   // profiling for Gaigen 2:
   //e3ga::g2Profiling::init();

   bivector iZ = _bivector(e1 ^ e2) ;
   bivector iX = _bivector(e2 ^ e3) ;

   mv_string_wedge = " \\wedge " ;
   mv_string_mul = " " ;
   mv_string_fp = "%2.0f" ;
//   mv_string_start = "{" ;
//   mv_string_end = "}" ;
   mv_basisVectorNames[0] = "\\mathbf{e}_1" ;
   mv_basisVectorNames[1] = "\\mathbf{e}_2" ;
   mv_basisVectorNames[2] = "\\mathbf{e}_3" ;

#if 0
   symbol CosPsi("\\cos(\\psi/2)") ;
   symbol IsinPsi("\\sin(\\psi/2)", -iZ) ;
   symbol CosTheta("\\cos(\\theta/2)") ;
   symbol IsinTheta("\\sin(\\theta/2)", -iX) ;
   symbol CosPhi("\\cos(\\phi/2)") ;
   symbol IsinPhi("\\sin(\\phi/2)", -iZ) ;
#else
   symbol CosPsi("C_\\psi") ;
   symbol IsinPsi("S_\\psi", -iZ) ;
   symbol CosTheta("C_\\theta") ;
   symbol IsinTheta("S_\\theta", -iX) ;
   symbol CosPhi("C_\\phi") ;
   symbol IsinPhi("S_\\phi", -iZ) ;
#endif

   sum R_psi( CosPsi ) ; R_psi += IsinPsi ;
   sum R_theta( CosTheta ) ; R_theta += IsinTheta ;
   sum R_phi( CosPhi ) ; R_phi += IsinPhi ;

#if 1
   sum Rl = (R_phi * R_theta) * R_psi ;
#elif 0
   sum Rl = R_phi * R_theta ;
#else
   sum Rl = R_phi ;
#endif
   sum Rr = Rl.reverse() ;

#if 0
   cout << "expect identity:" << endl ;
   sum ident = Rl * Rr ;
   ident.dump() ;
//   cout << "reduced: expect identity:" << endl ;
//   ident.reduce( false ) ;
//   ident.dump() ;
   cout << "reduced: expect identity:" << endl ;
   ident.reduce( ) ;
   ident.dump() ;
#endif

   symbol se1(e1) ;
   symbol se2(e2) ;
   symbol se3(e3) ;

#if 0
      sum t(Rl) ;
      t.reduce() ;
      cout << "R: " << endl ;
      t.dump() ;

      cout << "e1: " << endl ;
      se1.dump() ;

      cout << "R e1: " << endl ;
      t *= se1 ;
      //sum rot_e1 = t * Rr ;
      t.dump() ;

      cout << "reduced: R e1: " << endl ;
      t.reduce() ;
      t.dump() ;
#endif

#if 1
   {
//      cout << "R_{\\phi,z}(e_1):" << endl ;
      sum t(Rl) ;
      t *= se1 ;
      sum rot_e1 = t * Rr ;
      rot_e1.reduce() ;
//      rot_e1.dump() ;

      cout << "\n\n R_{11} &= \n\n" ;
      sum rot_e1_e1 = dot( rot_e1, e1 ) ;
      rot_e1_e1.reduce() ;
      rot_e1_e1.dump() ;

      cout << "\n\n R_{12} &= \n\n" ;
      sum rot_e1_e2 = dot( rot_e1, e2 ) ;
      rot_e1_e2.reduce() ;
      rot_e1_e2.dump() ;

      cout << "\n\n R_{13} &= \n\n" ;
      sum rot_e1_e3 = dot( rot_e1, e3 ) ;
      rot_e1_e3.reduce() ;
      rot_e1_e3.dump() ;
   }

   {
//      cout << "R_{\\phi,z}(e_2):" << endl ;
      sum t(Rl) ;
      t *= se2 ;
      sum rot_e2 = t * Rr ;
      rot_e2.reduce() ;
//      rot_e2.dump() ;

      cout << "\n\n R_{21} &= \n\n" ;
      sum rot_e2_e1 = dot( rot_e2, e1 ) ;
      rot_e2_e1.reduce() ;
      rot_e2_e1.dump() ;

      cout << "\n\n R_{22} &= \n\n" ;
      sum rot_e2_e2 = dot( rot_e2, e2 ) ;
      rot_e2_e2.reduce() ;
      rot_e2_e2.dump() ;

      cout << "\n\n R_{23} &= \n\n" ;
      sum rot_e2_e3 = dot( rot_e2, e3 ) ;
      rot_e2_e3.reduce() ;
      rot_e2_e3.dump() ;
   }

   {
//      cout << "R_{\\phi,z}(e_3):" << endl ;
      sum t(Rl) ;
      t *= se3 ;
      sum rot_e3 = t * Rr ;
      rot_e3.reduce() ;
//      rot_e3.dump() ;

      cout << "\n\n R_{31} &= \n\n" ;
      sum rot_e3_e1 = dot( rot_e3, e1 ) ;
      rot_e3_e1.reduce() ;
      rot_e3_e1.dump() ;

      cout << "\n\n R_{32} &= \n\n" ;
      sum rot_e3_e2 = dot( rot_e3, e2 ) ;
      rot_e3_e2.reduce() ;
      rot_e3_e2.dump() ;

      cout << "\n\n R_{33} &= \n\n" ;
      sum rot_e3_e3 = dot( rot_e3, e3 ) ;
      rot_e3_e3.reduce() ;
      rot_e3_e3.dump() ;

#if 0
      cout << "\n\n R_{33} (filtered) &= \n\n" ;
      rot_e3_e3.gradeFilter( SYMBOL_SCALAR_PART ) ;
      rot_e3_e3.dump() ;
#endif
   }
#endif

   return 0 ;
}
