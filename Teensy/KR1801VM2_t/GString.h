
/********************************************************************************
	Copyright 2013 Christopher Andrews (www.genx.biz)
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************/

#ifndef HEADER_GSTRING
	#define HEADER_STRING
	
	#include "Arduino.h"
	
	/*
		printf constants.
		
		Define these constants below to disable certain features.
		PRINTF_NO_FLOATING_POINT
		PRINTF_NO_WIDTH_PARAM
	*/
//#define		PRINTF_NO_FLOATING_POINT
//#define		PRINTF_NO_WIDTH_PARAM
	
	typedef unsigned char pft;
	
	#define CHAR_PERCENT				'%'
	#define CHAR_NULL					'\0'
	#define CHAR_MINUS					'-'
	#define CHAR_ZERO					'0'
	#define CHAR_NINE					'9'
	#define CHAR_SPACE					' '
	#define CHAR_STAR					'*'
	
	#define CHAR_s						's'
	#define CHAR_d						'd'
  #define CHAR_o            'o'
  #define CHAR_j            'j'
	#define CHAR_i						'i'
	#define CHAR_F						'F'
	#define CHAR_f						'f'
	#define CHAR_x						'x'
	#define CHAR_X						'X'
	#define CHAR_u						'u'
	#define CHAR_c						'c'
	#define CHAR_l						'l'
	
	#define PAD_RIGHT 					1
	#define PAD_ZERO					2	
	
	#define PRINTF_CONVERT_BUFFER_LEN	24
		
	class GString : public Print{

		public:
		
			/****************************************************************
				Constructor
					Pass in a pointer to any valid memory location.
			****************************************************************/
			
			template< typename T > 
				explicit GString( const T *u_DataPtr ) : Start( ( uint8_t* ) u_DataPtr ) , End( Start )
					{ return; }
					
		
			/****************************************************************
				Conversion operator ( char* )
					Allows accessing string elements directly using '[]' 
					operators or pointer arithmatic.
			****************************************************************/
			
			operator char*( void ) 
				{ 
					*End = '\0';
					return ( char* ) Start;
				}	
		
			/*****************************************************************
				operator +=
					String concatenation helper for 'String' like syntax.
			*****************************************************************/
			
			template< typename T > 
				GString &operator +=( const T &t )
					{
						print( t );
						return *this;
					}				
					
			/*****************************************************************
				clear function(s).
					Resets the string, if 'true' is passed in the data is set
					to zero rather than moving the distance pointer. 
			*****************************************************************/
			
			void clear( void ) { *( End = Start ) = 0x00; }
			
			void clear( bool b_Empty ) 
				{
					if( b_Empty ) while( --End > Start ){ *End = 0x00; }
					clear();
				}
				
					
			/****************************************************************
				concat function(s).
					String concatenation helpers for chained prints.
			****************************************************************/
			
			template< typename T > 
				GString &concat( const T &t )
					{
						print( t );
						return *this;
					}
					
			template< typename T > 
				GString &concat( const T &t, const int i )
					{
						print( t, i );
						return *this;
					}
					
					
			/****************************************************************
				count function.
					Length of printed data.
			****************************************************************/	
			
			size_t count( void ) const { return End - Start; }
			
			
			/****************************************************************
				end function.
					Adds a terminating null. Buffer will still increase after ending a string.
			****************************************************************/
			
			void end( void ) { *End++ = '\0'; }
			
			
			/****************************************************************
				find function.
					Find a character within the string.
					-1 returns on failure.
			****************************************************************/
			
			size_t find( const uint8_t &u_Char )
				{
					uint8_t	*u_Cursor = Start;
					while( ( *u_Cursor != u_Char ) && ( u_Cursor < End ) ) ++u_Cursor;
					return u_Cursor == End ? -1 : u_Cursor - Start; //Nothing found if reached End.
				}
				
				
			/****************************************************************
				printf function.
					Prints formatted text to the buffer.
					See sprintf at the bottom of the file for useage info.
			****************************************************************/
			
			pft printf( const char *format, ... )
				{
					va_list v_List;
					va_start( v_List, format );
					const pft p_Return = _printf( format, v_List );
					va_end( v_List );
					return p_Return;
				}

				
			/****************************************************************
				repeat function.
					Prints c_Character into the string repeating u_Count times.
			****************************************************************/
			
			void repeat( const char &c_Character, unsigned char u_Count ) { while( u_Count-- ) *End++ = c_Character; }
			

			/****************************************************************
				toLower function
					Converts the string into its lower case equivalent.
			****************************************************************/

			void toLower( void ) { translate( 'A', 'Z', 'a' ); }

				
			/****************************************************************
				toUpper function
					Converts the string into its upper case equivalent.
			****************************************************************/

			void toUpper( void ) { translate( 'a', 'z', 'A' ); }
			
			
			/****************************************************************
				translate function.
					Converts a range of characters to a different part 
					of the ascii table.
			****************************************************************/
			
			void translate( const uint8_t &u_StartLow, const uint8_t &u_StartHigh, const uint8_t &u_EndLow )
				{
					uint8_t	*u_Cursor = Start;
					
					while( u_Cursor < End ){
					
						if( ( u_StartLow <= *u_Cursor ) && ( *u_Cursor <= u_StartHigh ) ) *u_Cursor = u_EndLow + ( *u_Cursor - u_StartLow );
						++u_Cursor;
					}			
				}		
				
		protected:

			friend int sprintf( char * str, const char * format, ... );
			
			/****************************************************************
				assign function.
					Allows reseting the internal structure.
			****************************************************************/
							
			template< typename T >
				inline void assign( const T *u_DataPtr, const uint16_t u_Length )
					{
						Start = ( uint8_t* ) u_DataPtr;
						End = Start + u_Length;
					}			

			/****************************************************************
				write function(s).
					These are the virtual members provided by Print.
					Bytes are passed in and simply written into 'End'.
			****************************************************************/
			
			size_t write(const uint8_t *u_Buffer, size_t s_Size )
				{
					const size_t s_Return = s_Size;
					while( s_Size-- ) *End++ = *u_Buffer++;
					return s_Return;
				}
			
			size_t write( uint8_t u_Data )
				{ 
					*End++ = u_Data;
					return 0x01; 
				}
				
			//Specific for printf
			inline void write( uint8_t u_Data, pft &p_Counter )
				{ 
					*End++ = u_Data;
					++p_Counter;
				}
				
				
			/*
				printf runtime helper.
			*/
			
			pft _printf( const char *format, const va_list &v_List )
				{
					unsigned int				p_Width;
					pft				p_Pad;
					pft				p_Counter 		= 0x00;
					unsigned char 	buffer[ PRINTF_CONVERT_BUFFER_LEN ];			//Buffer for conversions.
					
					for( ; *format ; ++format ){
					
						if( *format == CHAR_PERCENT ){
						
							++format;
							p_Width = p_Pad = 0x00;
							
							bool b_LargeType = false;
							
							switch( *format ){
							
								case CHAR_NULL:		--format;						break;
								case CHAR_PERCENT:	write( *format, p_Counter );	break;
								case CHAR_MINUS:	p_Pad = PAD_RIGHT;
									
								default:
									
									//Check for '0' pad specifier.
									if( *format == CHAR_ZERO ){
										++format;
										p_Pad |= PAD_ZERO;
									}
									
									//Check for passed length.
									#ifndef PRINTF_NO_WIDTH_PARAM 
										if( *format == CHAR_STAR ){

											++format;
											p_Width = ( pft ) va_arg( v_List, int );
										
										//Width provided in format specifier.
										}else
									#endif
									
									//Calculate padding width.
									for ( ; *format >= CHAR_ZERO && *format <= CHAR_NINE; ++format ){
										p_Width *= 10;
										p_Width += *format - CHAR_ZERO;
									}
									
									char	c_PadChar = ( p_Pad & PAD_ZERO ) ? CHAR_ZERO : CHAR_SPACE;
									GString g_Convert( buffer );
									
									//Get length flag if for larger types.
									if( *format == CHAR_l ){
										b_LargeType = true;
										++format;
									}
									
									//Convert argument into a string.
									if( *format == CHAR_s ){													//String.
										char *c_Parameter = ( char* ) va_arg( v_List, int );
										g_Convert.assign( c_Parameter, strlen( c_Parameter ) );
										
                  }else if( *format == CHAR_o ||  *format == CHAR_j ){            //Signed octal integer.
                    if( b_LargeType ) g_Convert.print( va_arg( v_List, int32_t ), OCT );
                    else        g_Convert.print( ( int32_t ) va_arg( v_List, int ), OCT );
                    
									}else if( *format == CHAR_d ||  *format == CHAR_i ){ 						//Signed decimal integer.
										if( b_LargeType )	g_Convert.print( va_arg( v_List, int32_t ), DEC );
										else				g_Convert.print( ( int32_t ) va_arg( v_List, int ), DEC );
										
									}else if( *format == CHAR_u ){												//Unsigned integer.
										if( b_LargeType )	g_Convert.print( va_arg( v_List, uint32_t ), DEC );
										else				g_Convert.print( ( uint32_t ) va_arg( v_List, unsigned int ), DEC );
										
									#ifndef PRINTF_NO_FLOATING_POINT //Floating point conversions are expensive.
										}else if( *format == CHAR_f ){											//Floating point data.
											g_Convert.print( va_arg( v_List, double ) );
									#endif
									
									}else if( *format == CHAR_x ){												//Unsigned hexidecimal integer.
										if( b_LargeType )	g_Convert.print( va_arg( v_List, uint32_t ), HEX );
										else				g_Convert.print( ( uint32_t ) va_arg( v_List, uint16_t ), HEX );
										
									}else if(  *format == CHAR_c ){												//Character.
										*buffer = ( char ) va_arg( v_List, int );
										g_Convert.assign( buffer, 1 ); //This is cheaper than printing.
									}

									//Calculate remaining padding.
									const pft p_Len = g_Convert.count();
									if( p_Len >= p_Width )	p_Width = 0x00;
									else					p_Width -= p_Len;

									//Pad left if right justified.
									if( !( p_Pad & PAD_RIGHT ) ){
										repeat( c_PadChar, p_Width );
										p_Counter += p_Width;
										p_Width = 0;
									}
									
									p_Counter += write( ( unsigned char* ) ( char* ) g_Convert, p_Len );

									//Pad right if left justified.
									repeat( c_PadChar, p_Width );	
									p_Counter += p_Width;
							}
						}else //Standard character, write out.
							write( *format, p_Counter );
					}
					
					return p_Counter;
				}				
				
		private:
		
			uint8_t	*Start;
			uint8_t *End;
	};
	
	/****************************************************************
		sprintf function.
			Prints formatted text to a buffer.
			
			Formatting options use the following syntax: 
				%[flags][width][length]specifier
				
			Flags:
				-:	Left-justify within the given field width; Right justification is the default.
				0:	When padding is specified, zeros are used instead of spaces.
				
			Width:
				(number):	Minimum number of characters to be printed. 
							If the value to be printed is shorter than this number,
							the result is padded with blank spaces.
							The value is not truncated even if the result is larger.						
				
				*:	The width is not specified in the format string, 
					but as an additional integer value argument preceding 
					the argument that has to be formatted.
			Length:
				l:	d, i use long instead of int.
					u, x use unsigned long instead of unsigned int.
			Specifier:
				s:	String ( null terminated ).
				d:	Signed decimal integer ( 32bits max ).
				i:	Same as 'd'.
				u:	Unsigned decimal integer ( 32bits max ).
				f:	Decimal floating point number.
				x:	Unsigned decimal integer ( 32bits max ).
				c:	Character.
				%:	Escape character for printing '%'
			
	****************************************************************/
	
	inline int sprintf( char * str, const char * format, ... )
		{ 
			va_list v_List;
			va_start( v_List, format );
			GString g( str );
			int i_Return = g._printf( format, v_List );
			g.end();
			va_end( v_List );
			return i_Return;
		}

#endif
