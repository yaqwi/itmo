package pokemon;

import move.special.MudSlap;
import move.status.Confide;
import ru.ifmo.se.pokemon.Pokemon;
import ru.ifmo.se.pokemon.Type;

public class Trapinch extends Pokemon {
    public Trapinch(String name, int level) {
        super(name, level);
        setType(Type.GROUND);
        setStats(45, 100, 45, 45, 45, 10);
        setMove(new Confide(), new MudSlap());
    }
}
