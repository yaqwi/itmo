package pokemon;
import move.special.EnergyBall;
import move.special.Extrasensory;
import move.special.Swift;
import move.special.Thunder;
import ru.ifmo.se.pokemon.Pokemon;
import ru.ifmo.se.pokemon.Type;

public final class Mesprit extends Pokemon {
    public Mesprit(String name, int level) {
        super(name, level);
        setType(Type.PSYCHIC);
        setStats(80, 105, 105, 105, 105, 80);
        setMove(new Swift(), new Extrasensory(), new Thunder(), new EnergyBall());
    }
}
