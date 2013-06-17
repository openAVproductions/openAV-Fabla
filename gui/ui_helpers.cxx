



static void writeLoadSample(string pathname, LV2UI_Write_Function writeFunction, LV2UI_Controller controller) 
{
  
  
  LV2_Atom_Forge_Frame set_frame;
  LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank( forge, &set_frame, 1, uris->patch_Set);

  lv2_atom_forge_property_head(forge, uris->patch_body, 0);
  LV2_Atom_Forge_Frame body_frame;
  lv2_atom_forge_blank(forge, &body_frame, 2, 0);

  lv2_atom_forge_property_head(forge, uris->eg_file, 0);
  lv2_atom_forge_path(forge, filename, filename_len);

  lv2_atom_forge_pop(forge, &body_frame);
  lv2_atom_forge_pop(forge, &set_frame);
  
  
  
}
