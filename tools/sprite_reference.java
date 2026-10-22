import me.afoolslove.metalmaxre.*;
import me.afoolslove.metalmaxre.editors.*;
import me.afoolslove.metalmaxre.editors.text.mapping.CharMapCN;
import me.afoolslove.metalmaxre.helper.TileSetHelper;
import java.nio.file.*;
import java.util.*;

/** Editor-output diagnostic only. The authoritative host oracle executes
 * the ROM sprite routines in sprite_reference.py, including image50. */
class SpriteReference {
    public static void main(String[] args) throws Exception {
        try { run(args);System.exit(0); }
        catch (Throwable error) { error.printStackTrace();System.exit(1); }
    }
    private static void run(String[] args) throws Exception {
        RomBuffer rom=new RomBuffer(RomVersion.getChinese(),Path.of(args[0]));
        MetalMaxRe mm=new MetalMaxRe(rom);mm.setCharMap(new CharMapCN((String)null));
        EditorManagerImpl manager=new EditorManagerImpl(mm);mm.setEditorManager(manager);
        manager.registerDefaultEditors();manager.loadEditors().get();
        Path output=Path.of(args[1]);Files.createDirectories(output);
        // Image50 has no second left/right entry in the editor's217-byte
        // tables. Disable that incomplete row for this independent reference;
        // the importer records those poses as unavailable instead of reading
        // past the ROM-backed tables.
        var tileEditor=manager.<me.afoolslove.metalmaxre.editors.map.tileset.ITileSetEditor>getEditor(me.afoolslove.metalmaxre.editors.map.tileset.ITileSetEditor.class);
        tileEditor.getXA59E()[50]=0;
        for (int bank:new int[]{100,102,106,107,116,148,150,184,186,188}) {
            for (int frame=0;frame<2;frame++) {
                var colors=TileSetHelper.generateSpriteTileSet(mm,bank,frame!=0);
                StringBuilder data=new StringBuilder();
                for (int y=0;y<256;y++) {
                    for (int x=0;x<256;x++) {
                        data.append(colors[y][x]==null ? -1 : colors[y][x].toAwtColor().getRGB()&0xffffff).append('\n');
                    }
                }
                Files.writeString(output.resolve(String.format("bank-%02x-frame-%d.rgb",bank,frame)),data);
                System.out.println("REFERENCE "+bank+" "+frame);
            }
        }
        System.out.println("SPRITE_REFERENCE_OK");
    }
}
