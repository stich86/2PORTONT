------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--                      Terminal_Interface.Curses.Forms                     --
--                                                                          --
--                                 B O D Y                                  --
--                                                                          --
------------------------------------------------------------------------------
-- Copyright (c) 1998-2009,2011 Free Software Foundation, Inc.              --
--                                                                          --
-- Permission is hereby granted, free of charge, to any person obtaining a  --
-- copy of this software and associated documentation files (the            --
-- "Software"), to deal in the Software without restriction, including      --
-- without limitation the rights to use, copy, modify, merge, publish,      --
-- distribute, distribute with modifications, sublicense, and/or sell       --
-- copies of the Software, and to permit persons to whom the Software is    --
-- furnished to do so, subject to the following conditions:                 --
--                                                                          --
-- The above copyright notice and this permission notice shall be included  --
-- in all copies or substantial portions of the Software.                   --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  --
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               --
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   --
-- IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   --
-- DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    --
-- OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    --
-- THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               --
--                                                                          --
-- Except as contained in this notice, the name(s) of the above copyright   --
-- holders shall not be used in advertising or otherwise to promote the     --
-- sale, use or other dealings in this Software without prior written       --
-- authorization.                                                           --
------------------------------------------------------------------------------
--  Author:  Juergen Pfeifer, 1996
--  Version Control:
--  $Revision: 1.1 $
--  $Date: 2011/08/18 02:20:37 $
--  Binding Version 01.00
------------------------------------------------------------------------------
with Ada.Unchecked_Deallocation;
with Ada.Unchecked_Conversion;

with Interfaces.C; use Interfaces.C;
with Interfaces.C.Strings; use Interfaces.C.Strings;
with Interfaces.C.Pointers;

with Terminal_Interface.Curses.Aux;

package body Terminal_Interface.Curses.Forms is

   use Terminal_Interface.Curses.Aux;

   type C_Field_Array is array (Natural range <>) of aliased Field;
   package F_Array is new
     Interfaces.C.Pointers (Natural, Field, C_Field_Array, Null_Field);

------------------------------------------------------------------------------
   --  |
   --  |
   --  |
   --  subtype chars_ptr is Interfaces.C.Strings.chars_ptr;

   function FOS_2_CInt is new
     Ada.Unchecked_Conversion (Field_Option_Set,
                               C_Int);

   function CInt_2_FOS is new
     Ada.Unchecked_Conversion (C_Int,
                               Field_Option_Set);

   function FrmOS_2_CInt is new
     Ada.Unchecked_Conversion (Form_Option_Set,
                               C_Int);

   function CInt_2_FrmOS is new
     Ada.Unchecked_Conversion (C_Int,
                               Form_Option_Set);

   procedure Request_Name (Key  : Form_Request_Code;
                                Name : out String)
   is
      function Form_Request_Name (Key : C_Int) return chars_ptr;
      pragma Import (C, Form_Request_Name, "form_request_name");
   begin
      Fill_String (Form_Request_Name (C_Int (Key)), Name);
   end Request_Name;

   function Request_Name (Key : Form_Request_Code) return String
   is
      function Form_Request_Name (Key : C_Int) return chars_ptr;
      pragma Import (C, Form_Request_Name, "form_request_name");
   begin
      return Fill_String (Form_Request_Name (C_Int (Key)));
   end Request_Name;
------------------------------------------------------------------------------
   --  |
   --  |
   --  |
   --  |
   --  |=====================================================================
   --  | man page form_field_new.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   function Create (Height       : Line_Count;
                    Width        : Column_Count;
                    Top          : Line_Position;
                    Left         : Column_Position;
                    Off_Screen   : Natural := 0;
                    More_Buffers : Buffer_Number := Buffer_Number'First)
                    return Field
   is
      function Newfield (H, W, T, L, O, M : C_Int) return Field;
      pragma Import (C, Newfield, "new_field");
      Fld : constant Field := Newfield (C_Int (Height), C_Int (Width),
                                        C_Int (Top), C_Int (Left),
                                        C_Int (Off_Screen),
                                        C_Int (More_Buffers));
   begin
      if Fld = Null_Field then
         raise Form_Exception;
      end if;
      return Fld;
   end Create;
--  |
--  |
--  |
   procedure Delete (Fld : in out Field)
   is
      function Free_Field (Fld : Field) return C_Int;
      pragma Import (C, Free_Field, "free_field");

      Res : Eti_Error;
   begin
      Res := Free_Field (Fld);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
      Fld := Null_Field;
   end Delete;
   --  |
   --  |
   --  |
   function Duplicate (Fld  : Field;
                       Top  : Line_Position;
                       Left : Column_Position) return Field
   is
      function Dup_Field (Fld  : Field;
                          Top  : C_Int;
                          Left : C_Int) return Field;
      pragma Import (C, Dup_Field, "dup_field");

      F : constant Field := Dup_Field (Fld,
                                       C_Int (Top),
                                       C_Int (Left));
   begin
      if F = Null_Field then
         raise Form_Exception;
      end if;
      return F;
   end Duplicate;
   --  |
   --  |
   --  |
   function Link (Fld  : Field;
                  Top  : Line_Position;
                  Left : Column_Position) return Field
   is
      function Lnk_Field (Fld  : Field;
                          Top  : C_Int;
                          Left : C_Int) return Field;
      pragma Import (C, Lnk_Field, "link_field");

      F : constant Field := Lnk_Field (Fld,
                                       C_Int (Top),
                                       C_Int (Left));
   begin
      if F = Null_Field then
         raise Form_Exception;
      end if;
      return F;
   end Link;
   --  |
   --  |=====================================================================
   --  | man page form_field_just.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Justification (Fld  : Field;
                                Just : Field_Justification := None)
   is
      function Set_Field_Just (Fld  : Field;
                               Just : C_Int) return C_Int;
      pragma Import (C, Set_Field_Just, "set_field_just");

      Res : constant Eti_Error :=
        Set_Field_Just (Fld,
                        C_Int (Field_Justification'Pos (Just)));
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Justification;
   --  |
   --  |
   --  |
   function Get_Justification (Fld : Field) return Field_Justification
   is
      function Field_Just (Fld : Field) return C_Int;
      pragma Import (C, Field_Just, "field_just");
   begin
      return Field_Justification'Val (Field_Just (Fld));
   end Get_Justification;
   --  |
   --  |=====================================================================
   --  | man page form_field_buffer.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Buffer
     (Fld    : Field;
      Buffer : Buffer_Number := Buffer_Number'First;
      Str    : String)
   is
      type Char_Ptr is access all Interfaces.C.char;
      function Set_Fld_Buffer (Fld    : Field;
                                 Bufnum : C_Int;
                                 S      : Char_Ptr)
        return C_Int;
      pragma Import (C, Set_Fld_Buffer, "set_field_buffer");

      Txt : char_array (0 .. Str'Length);
      Len : size_t;
      Res : Eti_Error;
   begin
      To_C (Str, Txt, Len);
      Res := Set_Fld_Buffer (Fld, C_Int (Buffer), Txt (Txt'First)'Access);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Buffer;
   --  |
   --  |
   --  |
   procedure Get_Buffer
     (Fld    : Field;
      Buffer : Buffer_Number := Buffer_Number'First;
      Str    : out String)
   is
      function Field_Buffer (Fld : Field;
                             B   : C_Int) return chars_ptr;
      pragma Import (C, Field_Buffer, "field_buffer");
   begin
      Fill_String (Field_Buffer (Fld, C_Int (Buffer)), Str);
   end Get_Buffer;

   function Get_Buffer
     (Fld    : Field;
      Buffer : Buffer_Number := Buffer_Number'First) return String
   is
      function Field_Buffer (Fld : Field;
                             B   : C_Int) return chars_ptr;
      pragma Import (C, Field_Buffer, "field_buffer");
   begin
      return Fill_String (Field_Buffer (Fld, C_Int (Buffer)));
   end Get_Buffer;
   --  |
   --  |
   --  |
   procedure Set_Status (Fld    : Field;
                         Status : Boolean := True)
   is
      function Set_Fld_Status (Fld : Field;
                               St  : C_Int) return C_Int;
      pragma Import (C, Set_Fld_Status, "set_field_status");

      Res : constant Eti_Error := Set_Fld_Status (Fld, Boolean'Pos (Status));
   begin
      if Res /= E_Ok then
         raise Form_Exception;
      end if;
   end Set_Status;
   --  |
   --  |
   --  |
   function Changed (Fld : Field) return Boolean
   is
      function Field_Status (Fld : Field) return C_Int;
      pragma Import (C, Field_Status, "field_status");

      Res : constant C_Int := Field_Status (Fld);
   begin
      if Res = Curses_False then
         return False;
      else
         return True;
      end if;
   end Changed;
   --  |
   --  |
   --  |
   procedure Set_Maximum_Size (Fld : Field;
                               Max : Natural := 0)
   is
      function Set_Field_Max (Fld : Field;
                              M   : C_Int) return C_Int;
      pragma Import (C, Set_Field_Max, "set_max_field");

      Res : constant Eti_Error := Set_Field_Max (Fld, C_Int (Max));
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Maximum_Size;
   --  |
   --  |=====================================================================
   --  | man page form_field_opts.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Options (Fld     : Field;
                          Options : Field_Option_Set)
   is
      function Set_Field_Opts (Fld : Field;
                               Opt : C_Int) return C_Int;
      pragma Import (C, Set_Field_Opts, "set_field_opts");

      Opt : constant C_Int := FOS_2_CInt (Options);
      Res : Eti_Error;
   begin
      Res := Set_Field_Opts (Fld, Opt);
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Options;
   --  |
   --  |
   --  |
   procedure Switch_Options (Fld     : Field;
                             Options : Field_Option_Set;
                             On      : Boolean := True)
   is
      function Field_Opts_On (Fld : Field;
                              Opt : C_Int) return C_Int;
      pragma Import (C, Field_Opts_On, "field_opts_on");
      function Field_Opts_Off (Fld : Field;
                               Opt : C_Int) return C_Int;
      pragma Import (C, Field_Opts_Off, "field_opts_off");

      Err : Eti_Error;
      Opt : constant C_Int := FOS_2_CInt (Options);
   begin
      if On then
         Err := Field_Opts_On (Fld, Opt);
      else
         Err := Field_Opts_Off (Fld, Opt);
      end if;
      if Err /= E_Ok then
         Eti_Exception (Err);
      end if;
   end Switch_Options;
   --  |
   --  |
   --  |
   procedure Get_Options (Fld     : Field;
                          Options : out Field_Option_Set)
   is
      function Field_Opts (Fld : Field) return C_Int;
      pragma Import (C, Field_Opts, "field_opts");

      Res : constant C_Int := Field_Opts (Fld);
   begin
      Options := CInt_2_FOS (Res);
   end Get_Options;
   --  |
   --  |
   --  |
   function Get_Options (Fld : Field := Null_Field)
                         return Field_Option_Set
   is
      Fos : Field_Option_Set;
   begin
      Get_Options (Fld, Fos);
      return Fos;
   end Get_Options;
   --  |
   --  |=====================================================================
   --  | man page form_field_attributes.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Foreground
     (Fld   : Field;
      Fore  : Character_Attribute_Set := Normal_Video;
      Color : Color_Pair := Color_Pair'First)
   is
      function Set_Field_Fore (Fld  : Field;
                               Attr : C_Chtype) return C_Int;
      pragma Import (C, Set_Field_Fore, "set_field_fore");

      Ch : constant Attributed_Character := (Ch    => Character'First,
                                             Color => Color,
                                             Attr  => Fore);
      Res : constant Eti_Error :=
        Set_Field_Fore (Fld, AttrChar_To_Chtype (Ch));
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Foreground;
   --  |
   --  |
   --  |
   procedure Foreground (Fld  : Field;
                         Fore : out Character_Attribute_Set)
   is
      function Field_Fore (Fld : Field) return C_Chtype;
      pragma Import (C, Field_Fore, "field_fore");
   begin
      Fore := Chtype_To_AttrChar (Field_Fore (Fld)).Attr;
   end Foreground;

   procedure Foreground (Fld   : Field;
                         Fore  : out Character_Attribute_Set;
                         Color : out Color_Pair)
   is
      function Field_Fore (Fld : Field) return C_Chtype;
      pragma Import (C, Field_Fore, "field_fore");
   begin
      Fore  := Chtype_To_AttrChar (Field_Fore (Fld)).Attr;
      Color := Chtype_To_AttrChar (Field_Fore (Fld)).Color;
   end Foreground;
   --  |
   --  |
   --  |
   procedure Set_Background
     (Fld   : Field;
      Back  : Character_Attribute_Set := Normal_Video;
      Color : Color_Pair := Color_Pair'First)
   is
      function Set_Field_Back (Fld  : Field;
                               Attr : C_Chtype) return C_Int;
      pragma Import (C, Set_Field_Back, "set_field_back");

      Ch : constant Attributed_Character := (Ch    => Character'First,
                                             Color => Color,
                                             Attr  => Back);
      Res : constant Eti_Error :=
        Set_Field_Back (Fld, AttrChar_To_Chtype (Ch));
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Background;
   --  |
   --  |
   --  |
   procedure Background (Fld  : Field;
                         Back : out Character_Attribute_Set)
   is
      function Field_Back (Fld : Field) return C_Chtype;
      pragma Import (C, Field_Back, "field_back");
   begin
      Back := Chtype_To_AttrChar (Field_Back (Fld)).Attr;
   end Background;

   procedure Background (Fld   : Field;
                         Back  : out Character_Attribute_Set;
                         Color : out Color_Pair)
   is
      function Field_Back (Fld : Field) return C_Chtype;
      pragma Import (C, Field_Back, "field_back");
   begin
      Back  := Chtype_To_AttrChar (Field_Back (Fld)).Attr;
      Color := Chtype_To_AttrChar (Field_Back (Fld)).Color;
   end Background;
   --  |
   --  |
   --  |
   procedure Set_Pad_Character (Fld : Field;
                                Pad : Character := Space)
   is
      function Set_Field_Pad (Fld : Field;
                              Ch  : C_Int) return C_Int;
      pragma Import (C, Set_Field_Pad, "set_field_pad");

      Res : constant Eti_Error := Set_Field_Pad (Fld,
                                                 C_Int (Character'Pos (Pad)));
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Pad_Character;
   --  |
   --  |
   --  |
   procedure Pad_Character (Fld : Field;
                            Pad : out Character)
   is
      function Field_Pad (Fld : Field) return C_Int;
      pragma Import (C, Field_Pad, "field_pad");
   begin
      Pad := Character'Val (Field_Pad (Fld));
   end Pad_Character;
   --  |
   --  |=====================================================================
   --  | man page form_field_info.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Info (Fld                : Field;
                   Lines              : out Line_Count;
                   Columns            : out Column_Count;
                   First_Row          : out Line_Position;
                   First_Column       : out Column_Position;
                   Off_Screen         : out Natural;
                   Additional_Buffers : out Buffer_Number)
   is
      type C_Int_Access is access all C_Int;
      function Fld_Info (Fld : Field;
                         L, C, Fr, Fc, Os, Ab : C_Int_Access)
                         return C_Int;
      pragma Import (C, Fld_Info, "field_info");

      L, C, Fr, Fc, Os, Ab : aliased C_Int;
      Res : constant Eti_Error := Fld_Info (Fld,
                                            L'Access, C'Access,
                                            Fr'Access, Fc'Access,
                                            Os'Access, Ab'Access);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      else
         Lines              := Line_Count (L);
         Columns            := Column_Count (C);
         First_Row          := Line_Position (Fr);
         First_Column       := Column_Position (Fc);
         Off_Screen         := Natural (Os);
         Additional_Buffers := Buffer_Number (Ab);
      end if;
   end Info;
--  |
--  |
--  |
   procedure Dynamic_Info (Fld     : Field;
                           Lines   : out Line_Count;
                           Columns : out Column_Count;
                           Max     : out Natural)
   is
      type C_Int_Access is access all C_Int;
      function Dyn_Info (Fld : Field; L, C, M : C_Int_Access) return C_Int;
      pragma Import (C, Dyn_Info, "dynamic_field_info");

      L, C, M : aliased C_Int;
      Res : constant Eti_Error := Dyn_Info (Fld,
                                            L'Access, C'Access,
                                            M'Access);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      else
         Lines   := Line_Count (L);
         Columns := Column_Count (C);
         Max     := Natural (M);
      end if;
   end Dynamic_Info;
   --  |
   --  |=====================================================================
   --  | man page form_win.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Window (Frm : Form;
                         Win : Window)
   is
      function Set_Form_Win (Frm : Form;
                             Win : Window) return C_Int;
      pragma Import (C, Set_Form_Win, "set_form_win");

      Res : constant Eti_Error := Set_Form_Win (Frm, Win);
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Window;
   --  |
   --  |
   --  |
   function Get_Window (Frm : Form) return Window
   is
      function Form_Win (Frm : Form) return Window;
      pragma Import (C, Form_Win, "form_win");

      W : constant Window := Form_Win (Frm);
   begin
      return W;
   end Get_Window;
   --  |
   --  |
   --  |
   procedure Set_Sub_Window (Frm : Form;
                             Win : Window)
   is
      function Set_Form_Sub (Frm : Form;
                             Win : Window) return C_Int;
      pragma Import (C, Set_Form_Sub, "set_form_sub");

      Res : constant Eti_Error := Set_Form_Sub (Frm, Win);
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Sub_Window;
   --  |
   --  |
   --  |
   function Get_Sub_Window (Frm : Form) return Window
   is
      function Form_Sub (Frm : Form) return Window;
      pragma Import (C, Form_Sub, "form_sub");

      W : constant Window := Form_Sub (Frm);
   begin
      return W;
   end Get_Sub_Window;
   --  |
   --  |
   --  |
   procedure Scale (Frm     : Form;
                    Lines   : out Line_Count;
                    Columns : out Column_Count)
   is
      type C_Int_Access is access all C_Int;
      function M_Scale (Frm : Form; Yp, Xp : C_Int_Access) return C_Int;
      pragma Import (C, M_Scale, "scale_form");

      X, Y : aliased C_Int;
      Res  : constant Eti_Error := M_Scale (Frm, Y'Access, X'Access);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
      Lines := Line_Count (Y);
      Columns := Column_Count (X);
   end Scale;
   --  |
   --  |=====================================================================
   --  | man page menu_hook.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Field_Init_Hook (Frm  : Form;
                                  Proc : Form_Hook_Function)
   is
      function Set_Field_Init (Frm  : Form;
                               Proc : Form_Hook_Function) return C_Int;
      pragma Import (C, Set_Field_Init, "set_field_init");

      Res : constant Eti_Error := Set_Field_Init (Frm, Proc);
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Field_Init_Hook;
   --  |
   --  |
   --  |
   procedure Set_Field_Term_Hook (Frm  : Form;
                                  Proc : Form_Hook_Function)
   is
      function Set_Field_Term (Frm  : Form;
                               Proc : Form_Hook_Function) return C_Int;
      pragma Import (C, Set_Field_Term, "set_field_term");

      Res : constant Eti_Error := Set_Field_Term (Frm, Proc);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Field_Term_Hook;
   --  |
   --  |
   --  |
   procedure Set_Form_Init_Hook (Frm  : Form;
                                 Proc : Form_Hook_Function)
   is
      function Set_Form_Init (Frm  : Form;
                              Proc : Form_Hook_Function) return C_Int;
      pragma Import (C, Set_Form_Init, "set_form_init");

      Res : constant Eti_Error := Set_Form_Init (Frm, Proc);
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Form_Init_Hook;
   --  |
   --  |
   --  |
   procedure Set_Form_Term_Hook (Frm  : Form;
                                 Proc : Form_Hook_Function)
   is
      function Set_Form_Term (Frm  : Form;
                              Proc : Form_Hook_Function) return C_Int;
      pragma Import (C, Set_Form_Term, "set_form_term");

      Res : constant Eti_Error := Set_Form_Term (Frm, Proc);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Form_Term_Hook;
   --  |
   --  |=====================================================================
   --  | man page form_fields.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Redefine (Frm  : Form;
                       Flds : Field_Array_Access)
   is
      function Set_Frm_Fields (Frm   : Form;
                               Items : System.Address) return C_Int;
      pragma Import (C, Set_Frm_Fields, "set_form_fields");

      Res : Eti_Error;
   begin
      pragma Assert (Flds.all (Flds'Last) = Null_Field);
      if Flds.all (Flds'Last) /= Null_Field then
         raise Form_Exception;
      else
         Res := Set_Frm_Fields (Frm, Flds.all (Flds'First)'Address);
         if  Res /= E_Ok then
            Eti_Exception (Res);
         end if;
      end if;
   end Redefine;
   --  |
   --  |
   --  |
   function Fields (Frm   : Form;
                    Index : Positive) return Field
   is
      use F_Array;

      function C_Fields (Frm : Form) return Pointer;
      pragma Import (C, C_Fields, "form_fields");

      P : Pointer := C_Fields (Frm);
   begin
      if P = null or else Index > Field_Count (Frm) then
         raise Form_Exception;
      else
         P := P + ptrdiff_t (C_Int (Index) - 1);
         return P.all;
      end if;
   end Fields;
   --  |
   --  |
   --  |
   function Field_Count (Frm : Form) return Natural
   is
      function Count (Frm : Form) return C_Int;
      pragma Import (C, Count, "field_count");
   begin
      return Natural (Count (Frm));
   end Field_Count;
   --  |
   --  |
   --  |
   procedure Move (Fld    : Field;
                   Line   : Line_Position;
                   Column : Column_Position)
   is
      function Move (Fld : Field; L, C : C_Int) return C_Int;
      pragma Import (C, Move, "move_field");

      Res : constant Eti_Error := Move (Fld, C_Int (Line), C_Int (Column));
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Move;
   --  |
   --  |=====================================================================
   --  | man page form_new.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   function Create (Fields : Field_Array_Access) return Form
   is
      function NewForm (Fields : System.Address) return Form;
      pragma Import (C, NewForm, "new_form");

      M   : Form;
   begin
      pragma Assert (Fields.all (Fields'Last) = Null_Field);
      if Fields.all (Fields'Last) /= Null_Field then
         raise Form_Exception;
      else
         M := NewForm (Fields.all (Fields'First)'Address);
         if M = Null_Form then
            raise Form_Exception;
         end if;
         return M;
      end if;
   end Create;
   --  |
   --  |
   --  |
   procedure Delete (Frm : in out Form)
   is
      function Free (Frm : Form) return C_Int;
      pragma Import (C, Free, "free_form");

      Res : constant Eti_Error := Free (Frm);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
      Frm := Null_Form;
   end Delete;
   --  |
   --  |=====================================================================
   --  | man page form_opts.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Options (Frm     : Form;
                          Options : Form_Option_Set)
   is
      function Set_Form_Opts (Frm : Form;
                              Opt : C_Int) return C_Int;
      pragma Import (C, Set_Form_Opts, "set_form_opts");

      Opt : constant C_Int := FrmOS_2_CInt (Options);
      Res : Eti_Error;
   begin
      Res := Set_Form_Opts (Frm, Opt);
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Options;
   --  |
   --  |
   --  |
   procedure Switch_Options (Frm     : Form;
                             Options : Form_Option_Set;
                             On      : Boolean := True)
   is
      function Form_Opts_On (Frm : Form;
                             Opt : C_Int) return C_Int;
      pragma Import (C, Form_Opts_On, "form_opts_on");
      function Form_Opts_Off (Frm : Form;
                              Opt : C_Int) return C_Int;
      pragma Import (C, Form_Opts_Off, "form_opts_off");

      Err : Eti_Error;
      Opt : constant C_Int := FrmOS_2_CInt (Options);
   begin
      if On then
         Err := Form_Opts_On (Frm, Opt);
      else
         Err := Form_Opts_Off (Frm, Opt);
      end if;
      if Err /= E_Ok then
         Eti_Exception (Err);
      end if;
   end Switch_Options;
   --  |
   --  |
   --  |
   procedure Get_Options (Frm     : Form;
                          Options : out Form_Option_Set)
   is
      function Form_Opts (Frm : Form) return C_Int;
      pragma Import (C, Form_Opts, "form_opts");

      Res : constant C_Int := Form_Opts (Frm);
   begin
      Options := CInt_2_FrmOS (Res);
   end Get_Options;
   --  |
   --  |
   --  |
   function Get_Options (Frm : Form := Null_Form) return Form_Option_Set
   is
      Fos : Form_Option_Set;
   begin
      Get_Options (Frm, Fos);
      return Fos;
   end Get_Options;
   --  |
   --  |=====================================================================
   --  | man page form_post.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Post (Frm  : Form;
                   Post : Boolean := True)
   is
      function M_Post (Frm : Form) return C_Int;
      pragma Import (C, M_Post, "post_form");
      function M_Unpost (Frm : Form) return C_Int;
      pragma Import (C, M_Unpost, "unpost_form");

      Res : Eti_Error;
   begin
      if Post then
         Res := M_Post (Frm);
      else
         Res := M_Unpost (Frm);
      end if;
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Post;
   --  |
   --  |=====================================================================
   --  | man page form_cursor.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Position_Cursor (Frm : Form)
   is
      function Pos_Form_Cursor (Frm : Form) return C_Int;
      pragma Import (C, Pos_Form_Cursor, "pos_form_cursor");

      Res : constant Eti_Error := Pos_Form_Cursor (Frm);
   begin
      if  Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Position_Cursor;
   --  |
   --  |=====================================================================
   --  | man page form_data.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   function Data_Ahead (Frm : Form) return Boolean
   is
      function Ahead (Frm : Form) return C_Int;
      pragma Import (C, Ahead, "data_ahead");

      Res : constant C_Int := Ahead (Frm);
   begin
      if Res = Curses_False then
         return False;
      else
         return True;
      end if;
   end Data_Ahead;
   --  |
   --  |
   --  |
   function Data_Behind (Frm : Form) return Boolean
   is
      function Behind (Frm : Form) return C_Int;
      pragma Import (C, Behind, "data_behind");

      Res : constant C_Int := Behind (Frm);
   begin
      if Res = Curses_False then
         return False;
      else
         return True;
      end if;
   end Data_Behind;
   --  |
   --  |=====================================================================
   --  | man page form_driver.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   function Driver (Frm : Form;
                    Key : Key_Code) return Driver_Result
   is
      function Frm_Driver (Frm : Form; Key : C_Int) return C_Int;
      pragma Import (C, Frm_Driver, "form_driver");

      R : constant Eti_Error := Frm_Driver (Frm, C_Int (Key));
   begin
      if R /= E_Ok then
         if R = E_Unknown_Command then
            return Unknown_Request;
         elsif R = E_Invalid_Field then
            return Invalid_Field;
         elsif R = E_Request_Denied then
            return Request_Denied;
         else
            Eti_Exception (R);
            return Form_Ok;
         end if;
      else
         return Form_Ok;
      end if;
   end Driver;
   --  |
   --  |=====================================================================
   --  | man page form_page.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_Current (Frm : Form;
                          Fld : Field)
   is
      function Set_Current_Fld (Frm : Form; Fld : Field) return C_Int;
      pragma Import (C, Set_Current_Fld, "set_current_field");

      Res : constant Eti_Error := Set_Current_Fld (Frm, Fld);
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Current;
   --  |
   --  |
   --  |
   function Current (Frm : Form) return Field
   is
      function Current_Fld (Frm : Form) return Field;
      pragma Import (C, Current_Fld, "current_field");

      Fld : constant Field := Current_Fld (Frm);
   begin
      if Fld = Null_Field then
         raise Form_Exception;
      end if;
      return Fld;
   end Current;
   --  |
   --  |
   --  |
   procedure Set_Page (Frm  : Form;
                       Page : Page_Number := Page_Number'First)
   is
      function Set_Frm_Page (Frm : Form; Pg : C_Int) return C_Int;
      pragma Import (C, Set_Frm_Page, "set_form_page");

      Res : constant Eti_Error := Set_Frm_Page (Frm, C_Int (Page));
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_Page;
   --  |
   --  |
   --  |
   function Page (Frm : Form) return Page_Number
   is
      function Get_Page (Frm : Form) return C_Int;
      pragma Import (C, Get_Page, "form_page");

      P : constant C_Int := Get_Page (Frm);
   begin
      if P < 0 then
         raise Form_Exception;
      else
         return Page_Number (P);
      end if;
   end Page;

   function Get_Index (Fld : Field) return Positive
   is
      function Get_Fieldindex (Fld : Field) return C_Int;
      pragma Import (C, Get_Fieldindex, "field_index");

      Res : constant C_Int := Get_Fieldindex (Fld);
   begin
      if Res = Curses_Err then
         raise Form_Exception;
      end if;
      return Positive (Natural (Res) + Positive'First);
   end Get_Index;

   --  |
   --  |=====================================================================
   --  | man page form_new_page.3x
   --  |=====================================================================
   --  |
   --  |
   --  |
   procedure Set_New_Page (Fld      : Field;
                           New_Page : Boolean := True)
   is
      function Set_Page (Fld : Field; Flg : C_Int) return C_Int;
      pragma Import (C, Set_Page, "set_new_page");

      Res : constant Eti_Error := Set_Page (Fld, Boolean'Pos (New_Page));
   begin
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
   end Set_New_Page;
   --  |
   --  |
   --  |
   function Is_New_Page (Fld : Field) return Boolean
   is
      function Is_New (Fld : Field) return C_Int;
      pragma Import (C, Is_New, "new_page");

      Res : constant C_Int := Is_New (Fld);
   begin
      if Res = Curses_False then
         return False;
      else
         return True;
      end if;
   end Is_New_Page;

   procedure Free (FA          : in out Field_Array_Access;
                   Free_Fields : Boolean := False)
   is
      procedure Release is new Ada.Unchecked_Deallocation
        (Field_Array, Field_Array_Access);
   begin
      if FA /= null and then Free_Fields then
         for I in FA'First .. (FA'Last - 1) loop
            if FA.all (I) /= Null_Field then
               Delete (FA.all (I));
            end if;
         end loop;
      end if;
      Release (FA);
   end Free;

   --  |=====================================================================

   function Default_Field_Options return Field_Option_Set
   is
   begin
      return Get_Options (Null_Field);
   end Default_Field_Options;

   function Default_Form_Options return Form_Option_Set
   is
   begin
      return Get_Options (Null_Form);
   end Default_Form_Options;

end Terminal_Interface.Curses.Forms;
