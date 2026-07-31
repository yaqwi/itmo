package pokemon;

import move.physical.Scratch;
import move.special.Blizzard;
import move.status.TailWhip;
import ru.ifmo.se.pokemon.Pokemon;
import ru.ifmo.se.pokemon.Type;

public class Psyduck extends Pokemon {
    public Psyduck(String name, int level) {
        super(name, level);
        setType(Type.WATER);
        setStats(50, 52, 48, 65, 50, 55);
        setMove(new Scratch(), new Blizzard(), new TailWhip());
    }
}
